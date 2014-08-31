#include "base64.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* Encoding table as described in RFC1113. */
const static uint8_t b64_encode_table[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz0123456789+/";

/* Decoding table. */
const static int8_t b64_decode_table[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 00-0F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 10-1F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,	/* 20-2F */
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,	/* 30-3F */
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /* 40-4F */
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,	/* 50-5F */
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,	/* 60-6F */
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,	/* 70-7F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 80-8F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 90-9F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* A0-AF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* B0-BF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* C0-CF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* D0-DF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* E0-EF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1	/* F0-FF */
};

uint8_t *base64decode(const uint8_t *src, const size_t buflen, size_t *retlen)
{
	int8_t digit, lastdigit;
	size_t i, j;
	uint8_t *decoded;
	const size_t maxlen = ((buflen + 3) / 4) * 3;

	/* Sanity check */
	assert(src != NULL);

	digit = lastdigit = j = 0;
	decoded = malloc(maxlen + 1);
	if (decoded == NULL) return NULL;
	for (i = 0; i < buflen; ++i) {
		if ((digit = b64_decode_table[src[i]]) != -1) {
			/* Decode block */
			switch (i % 4) {
				case 1:
					decoded[j++] = ((lastdigit << 2) | ((digit & 0x30) >> 4));
					break;
				case 2:
					decoded[j++] = (((lastdigit & 0xF) << 4) | ((digit & 0x3C) >> 2));
					break;
				case 3:
					decoded[j++] = (((lastdigit & 0x03) << 6) | digit);
					break;
			}
			lastdigit = digit;
		}
	}

	if (retlen != NULL) *retlen = j;
	decoded[j] = '\0';
	return decoded; /* Must be free()'d by caller */
}

uint8_t *base64encode(const uint8_t *src, const size_t buflen, size_t *retlen)
{
	size_t i, j;
	const size_t maxlen = (((buflen + 3) & ~3)) * 4;
	uint8_t *encoded = malloc(maxlen + 1);
	if (encoded == NULL) return NULL;

	/* Sanity check */
	assert(src != NULL);
	assert(buflen > 0);

	j = 0;
	for (i = 0; i < buflen + 1; ++i) {
		/* Encode block */
		switch (i % 3) {
			case 0:
				encoded[j++] = b64_encode_table[src[i] >> 2];
				encoded[j++] = b64_encode_table[((src[i] & 0x03) << 4) |
				                                ((src[i + 1] & 0xF0) >> 4)];
				break;
			case 1:
				encoded[j++] = b64_encode_table[((src[i] & 0x0F) << 2) |
				                                ((src[i + 1] & 0xC0) >> 6)];
				break;
			case 2:
				encoded[j++] = b64_encode_table[(src[i] & 0x3F)];
				break;
		}
	}

	/* Add padding if necessary */
	if ((j % 4) != 0) {
		const size_t with_padding = ((j + 3) & ~3); /* Align to 4 bytes */
		do {
			encoded[j++] = '=';
		} while (j < with_padding);
	}

	assert(j <= maxlen);

	if (retlen != NULL) *retlen = j;
	encoded[j] = '\0';
	return encoded; /* Must be free()'d by caller */
}
