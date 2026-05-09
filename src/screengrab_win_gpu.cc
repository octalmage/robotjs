#include "screengrab.h"

#if defined(IS_WINDOWS)

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

template <typename T>
static void safeRelease(T **object)
{
	if (*object != NULL) {
		(*object)->Release();
		*object = NULL;
	}
}

struct CaptureState {
	ID3D11Device *device;
	ID3D11DeviceContext *context;
	IDXGIOutputDuplication *duplication;
	ID3D11Texture2D *latestFrame;
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11SamplerState *sampler;
	ID3D11Texture2D *scaledTexture;
	ID3D11RenderTargetView *scaledTarget;
	ID3D11Texture2D *stagingTexture;
	int sourceWidth;
	int sourceHeight;
	int destWidth;
	int destHeight;
};

static CaptureState gState = {};
static char gLastError[256] = "not attempted";

static HRESULT failWithHr(const char *step, HRESULT hr)
{
	snprintf(gLastError,
	         sizeof(gLastError),
	         "%s failed with HRESULT 0x%08lx",
	         step,
	         (unsigned long)hr);
	return hr;
}

static void failWithMessage(const char *message)
{
	strncpy(gLastError, message, sizeof(gLastError) - 1);
	gLastError[sizeof(gLastError) - 1] = '\0';
}

extern "C" const char *copyMMBitmapFromDisplayInRectWindowsGpuLastError(void)
{
	return gLastError;
}

static void releaseFrameResources(void)
{
	safeRelease(&gState.stagingTexture);
	safeRelease(&gState.scaledTarget);
	safeRelease(&gState.scaledTexture);
	gState.destWidth = 0;
	gState.destHeight = 0;
}

static void releaseCaptureState(void)
{
	releaseFrameResources();
	safeRelease(&gState.sampler);
	safeRelease(&gState.pixelShader);
	safeRelease(&gState.vertexShader);
	safeRelease(&gState.latestFrame);
	safeRelease(&gState.duplication);
	safeRelease(&gState.context);
	safeRelease(&gState.device);
	gState.sourceWidth = 0;
	gState.sourceHeight = 0;
}

static HRESULT compileShader(const char *source,
                             const char *entry,
                             const char *target,
                             ID3DBlob **shader)
{
	ID3DBlob *errors = NULL;
	HRESULT hr = D3DCompile(source,
	                        strlen(source),
	                        NULL,
	                        NULL,
	                        NULL,
	                        entry,
	                        target,
	                        D3DCOMPILE_OPTIMIZATION_LEVEL3,
	                        0,
	                        shader,
	                        &errors);
	safeRelease(&errors);
	return FAILED(hr) ? failWithHr("D3DCompile", hr) : hr;
}

static HRESULT createShadersAndSampler(void)
{
	static const char *vertexShaderSource =
		"struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };"
		"VSOut main(uint id : SV_VertexID) {"
		"	float2 pos[3] = { float2(-1.0, -1.0), float2(-1.0, 3.0), float2(3.0, -1.0) };"
		"	VSOut output;"
		"	output.pos = float4(pos[id], 0.0, 1.0);"
		"	output.uv = float2((pos[id].x + 1.0) * 0.5, (1.0 - pos[id].y) * 0.5);"
		"	return output;"
		"}";
	static const char *pixelShaderSource =
		"Texture2D screenTexture : register(t0);"
		"float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET {"
		"	uint width;"
		"	uint height;"
		"	screenTexture.GetDimensions(width, height);"
		"	uint x = min((uint)(uv.x * width), width - 1);"
		"	uint y = min((uint)(uv.y * height), height - 1);"
		"	return screenTexture.Load(int3(x, y, 0));"
		"}";

	ID3DBlob *vertexShader = NULL;
	ID3DBlob *pixelShader = NULL;
	D3D11_SAMPLER_DESC samplerDesc;
	HRESULT hr;

	hr = compileShader(vertexShaderSource, "main", "vs_4_0", &vertexShader);
	if (FAILED(hr)) return hr;

	hr = gState.device->CreateVertexShader(vertexShader->GetBufferPointer(),
	                                       vertexShader->GetBufferSize(),
	                                       NULL,
	                                       &gState.vertexShader);
	if (FAILED(hr)) {
		safeRelease(&vertexShader);
		return failWithHr("CreateVertexShader", hr);
	}

	hr = compileShader(pixelShaderSource, "main", "ps_4_0", &pixelShader);
	if (FAILED(hr)) {
		safeRelease(&vertexShader);
		return hr;
	}

	hr = gState.device->CreatePixelShader(pixelShader->GetBufferPointer(),
	                                      pixelShader->GetBufferSize(),
	                                      NULL,
	                                      &gState.pixelShader);
	safeRelease(&vertexShader);
	safeRelease(&pixelShader);
	if (FAILED(hr)) return failWithHr("CreatePixelShader", hr);

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = gState.device->CreateSamplerState(&samplerDesc, &gState.sampler);
	return FAILED(hr) ? failWithHr("CreateSamplerState", hr) : hr;
}

static HRESULT initializeCaptureState(void)
{
	IDXGIFactory1 *factory = NULL;
	IDXGIAdapter1 *adapter = NULL;
	IDXGIAdapter1 *selectedAdapter = NULL;
	IDXGIOutput *output = NULL;
	IDXGIOutput *selectedOutput = NULL;
	IDXGIOutput1 *output1 = NULL;
	DXGI_OUTPUT_DESC outputDesc;
	DXGI_OUTPUT_DESC selectedOutputDesc;
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr;
	UINT adapterIndex;
	UINT outputIndex;

	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)&factory);
	if (FAILED(hr)) return failWithHr("CreateDXGIFactory1", hr);

	ZeroMemory(&selectedOutputDesc, sizeof(selectedOutputDesc));
	for (adapterIndex = 0;
	     factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND;
	     ++adapterIndex) {
		for (outputIndex = 0;
		     adapter->EnumOutputs(outputIndex, &output) != DXGI_ERROR_NOT_FOUND;
		     ++outputIndex) {
			if (SUCCEEDED(output->GetDesc(&outputDesc)) &&
			    outputDesc.AttachedToDesktop) {
				if (selectedOutput == NULL) {
					selectedAdapter = adapter;
					selectedAdapter->AddRef();
					selectedOutput = output;
					selectedOutput->AddRef();
					selectedOutputDesc = outputDesc;
				}
				if (outputDesc.DesktopCoordinates.left <= 0 &&
				    outputDesc.DesktopCoordinates.top <= 0 &&
				    outputDesc.DesktopCoordinates.right > 0 &&
				    outputDesc.DesktopCoordinates.bottom > 0) {
					safeRelease(&selectedAdapter);
					safeRelease(&selectedOutput);
					selectedAdapter = adapter;
					selectedAdapter->AddRef();
					selectedOutput = output;
					selectedOutput->AddRef();
					selectedOutputDesc = outputDesc;
					safeRelease(&output);
					break;
				}
			}
			safeRelease(&output);
		}
		safeRelease(&adapter);
		if (selectedOutput != NULL &&
		    selectedOutputDesc.DesktopCoordinates.left <= 0 &&
		    selectedOutputDesc.DesktopCoordinates.top <= 0 &&
		    selectedOutputDesc.DesktopCoordinates.right > 0 &&
		    selectedOutputDesc.DesktopCoordinates.bottom > 0) {
			break;
		}
	}

	if (selectedAdapter == NULL || selectedOutput == NULL) {
		hr = DXGI_ERROR_NOT_FOUND;
	} else {
		hr = D3D11CreateDevice(selectedAdapter,
		                       D3D_DRIVER_TYPE_UNKNOWN,
		                       NULL,
		                       D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		                       featureLevels,
		                       ARRAYSIZE(featureLevels),
		                       D3D11_SDK_VERSION,
		                       &gState.device,
		                       &featureLevel,
		                       &gState.context);
	}
	if (SUCCEEDED(hr)) {
		hr = selectedOutput->QueryInterface(__uuidof(IDXGIOutput1), (void **)&output1);
	}
	if (SUCCEEDED(hr)) {
		hr = output1->DuplicateOutput(gState.device, &gState.duplication);
	}
	if (SUCCEEDED(hr)) {
		gState.sourceWidth = selectedOutputDesc.DesktopCoordinates.right -
		                     selectedOutputDesc.DesktopCoordinates.left;
		gState.sourceHeight = selectedOutputDesc.DesktopCoordinates.bottom -
		                      selectedOutputDesc.DesktopCoordinates.top;
		hr = createShadersAndSampler();
	}

	safeRelease(&output1);
	safeRelease(&selectedOutput);
	safeRelease(&selectedAdapter);
	safeRelease(&factory);

	if (FAILED(hr)) {
		failWithHr("initializeCaptureState", hr);
		releaseCaptureState();
	}
	return hr;
}

static HRESULT ensureCaptureState(void)
{
	if (gState.device != NULL && gState.duplication != NULL) {
		return S_OK;
	}

	return initializeCaptureState();
}

static HRESULT ensureDestinationTextures(int width, int height)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hr;

	if (gState.scaledTexture != NULL &&
	    gState.stagingTexture != NULL &&
	    gState.destWidth == width &&
	    gState.destHeight == height) {
		return S_OK;
	}

	releaseFrameResources();

	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = (UINT)width;
	textureDesc.Height = (UINT)height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;

	hr = gState.device->CreateTexture2D(&textureDesc, NULL, &gState.scaledTexture);
	if (FAILED(hr)) return failWithHr("CreateTexture2D scaled", hr);

	hr = gState.device->CreateRenderTargetView(gState.scaledTexture,
	                                           NULL,
	                                           &gState.scaledTarget);
	if (FAILED(hr)) return failWithHr("CreateRenderTargetView", hr);

	textureDesc.Usage = D3D11_USAGE_STAGING;
	textureDesc.BindFlags = 0;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	hr = gState.device->CreateTexture2D(&textureDesc, NULL, &gState.stagingTexture);
	if (FAILED(hr)) return failWithHr("CreateTexture2D staging", hr);

	gState.destWidth = width;
	gState.destHeight = height;
	return S_OK;
}

static HRESULT ensureLatestFrame(void)
{
	IDXGIResource *resource = NULL;
	ID3D11Texture2D *frame = NULL;
	DXGI_OUTDUPL_FRAME_INFO frameInfo;
	D3D11_TEXTURE2D_DESC frameDesc;
	HRESULT hr;

	hr = gState.duplication->AcquireNextFrame(gState.latestFrame == NULL ? 16 : 0,
	                                          &frameInfo,
	                                          &resource);
	if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
		if (gState.latestFrame == NULL) {
			failWithHr("AcquireNextFrame", hr);
		}
		return gState.latestFrame != NULL ? S_OK : hr;
	}
	if (FAILED(hr)) {
		if (hr == DXGI_ERROR_ACCESS_LOST ||
		    hr == DXGI_ERROR_DEVICE_REMOVED ||
		    hr == DXGI_ERROR_DEVICE_RESET) {
			releaseCaptureState();
		}
		failWithHr("AcquireNextFrame", hr);
		return hr;
	}

	hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void **)&frame);
	if (SUCCEEDED(hr)) {
		frame->GetDesc(&frameDesc);

		if (gState.latestFrame == NULL ||
		    gState.sourceWidth != (int)frameDesc.Width ||
		    gState.sourceHeight != (int)frameDesc.Height) {
			safeRelease(&gState.latestFrame);
			frameDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			frameDesc.CPUAccessFlags = 0;
			frameDesc.MiscFlags = 0;
			frameDesc.Usage = D3D11_USAGE_DEFAULT;
			hr = gState.device->CreateTexture2D(&frameDesc, NULL, &gState.latestFrame);
			if (SUCCEEDED(hr)) {
				gState.sourceWidth = (int)frameDesc.Width;
				gState.sourceHeight = (int)frameDesc.Height;
			} else {
				failWithHr("CreateTexture2D latest frame", hr);
			}
		}
		if (SUCCEEDED(hr)) {
			gState.context->CopyResource(gState.latestFrame, frame);
		}
	} else {
		failWithHr("QueryInterface ID3D11Texture2D", hr);
	}

	safeRelease(&frame);
	safeRelease(&resource);
	gState.duplication->ReleaseFrame();
	return hr;
}

static HRESULT renderScaledFrame(void)
{
	ID3D11ShaderResourceView *frameView = NULL;
	D3D11_VIEWPORT viewport;
	HRESULT hr;

	hr = gState.device->CreateShaderResourceView(gState.latestFrame,
	                                             NULL,
	                                             &frameView);
	if (FAILED(hr)) return failWithHr("CreateShaderResourceView", hr);

	ZeroMemory(&viewport, sizeof(viewport));
	viewport.Width = (FLOAT)gState.destWidth;
	viewport.Height = (FLOAT)gState.destHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	gState.context->IASetInputLayout(NULL);
	gState.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gState.context->VSSetShader(gState.vertexShader, NULL, 0);
	gState.context->PSSetShader(gState.pixelShader, NULL, 0);
	gState.context->PSSetShaderResources(0, 1, &frameView);
	gState.context->PSSetSamplers(0, 1, &gState.sampler);
	gState.context->RSSetViewports(1, &viewport);
	gState.context->OMSetRenderTargets(1, &gState.scaledTarget, NULL);
	gState.context->Draw(3, 0);

	ID3D11ShaderResourceView *nullView = NULL;
	gState.context->PSSetShaderResources(0, 1, &nullView);
	gState.context->CopyResource(gState.stagingTexture, gState.scaledTexture);

	safeRelease(&frameView);
	return S_OK;
}

static bool bufferLooksBlack(const uint8_t *buffer, size_t width, size_t height, size_t byteWidth)
{
	const size_t xStep = width > 64 ? width / 64 : 1;
	const size_t yStep = height > 64 ? height / 64 : 1;

	for (size_t y = 0; y < height; y += yStep) {
		const uint8_t *row = buffer + y * byteWidth;
		for (size_t x = 0; x < width; x += xStep) {
			const uint8_t *pixel = row + x * 4;
			if (pixel[0] != 0 || pixel[1] != 0 || pixel[2] != 0) {
				return false;
			}
		}
	}

	return true;
}

extern "C" MMBitmapRef copyMMBitmapFromDisplayInRectWindowsGpu(MMRect rect)
{
	MMBitmapRef bitmap = NULL;
	uint8_t *buffer = NULL;
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr;
	const int width = (int)rect.size.width;
	const int height = (int)rect.size.height;
	const size_t byteWidth = 4 * rect.size.width;

	if (width <= 0 || height <= 0) {
		return NULL;
	}

	hr = ensureCaptureState();
	if (FAILED(hr)) return NULL;

	hr = ensureDestinationTextures(width, height);
	if (FAILED(hr)) return NULL;

	for (int attempt = 0; attempt < 3; ++attempt) {
		hr = ensureLatestFrame();
		if (FAILED(hr)) return NULL;

		hr = renderScaledFrame();
		if (FAILED(hr)) return NULL;

		hr = gState.context->Map(gState.stagingTexture,
		                         0,
		                         D3D11_MAP_READ,
		                         0,
		                         &mapped);
		if (FAILED(hr)) {
			failWithHr("Map staging texture", hr);
			return NULL;
		}

		buffer = (uint8_t *)malloc(byteWidth * rect.size.height);
		if (buffer != NULL) {
			for (size_t y = 0; y < rect.size.height; ++y) {
				memcpy(buffer + y * byteWidth,
				       (uint8_t *)mapped.pData + y * mapped.RowPitch,
				       byteWidth);
			}
		}
		gState.context->Unmap(gState.stagingTexture, 0);

		if (buffer == NULL) {
			failWithMessage("malloc failed for GPU readback buffer");
			return NULL;
		}
		if (!bufferLooksBlack(buffer, rect.size.width, rect.size.height, byteWidth)) {
			break;
		}

		free(buffer);
		buffer = NULL;
		safeRelease(&gState.latestFrame);
		Sleep(16);
	}
	if (buffer == NULL) {
		failWithMessage("GPU capture produced an all-black frame");
		return NULL;
	}

	bitmap = createMMBitmap(buffer,
	                        rect.size.width,
	                        rect.size.height,
	                        byteWidth,
	                        32,
	                        4);
	if (bitmap == NULL) {
		free(buffer);
	}

	return bitmap;
}

#endif
