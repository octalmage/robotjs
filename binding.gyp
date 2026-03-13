{
  'variables': {
    'robotjs_enable_png%': '<!(node -p "process.env.ROBOTJS_ENABLE_PNG === \'1\' ? 1 : 0")'
  },
  'targets': [{
    'target_name': 'robotjs',
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': { 'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.15',
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
    'include_dirs': [
      '<!(node -p "require(\'node-addon-api\').include_dir")',
    ],
    
    'conditions': [
      ['OS == "mac"', {
        'include_dirs': [
          '<!(node -p "require(\'node-addon-api\').include_dir")',
          'System/Library/Frameworks/CoreFoundation.Framework/Headers',
          'System/Library/Frameworks/Carbon.Framework/Headers',
          'System/Library/Frameworks/ApplicationServices.framework/Headers',
          'System/Library/Frameworks/OpenGL.framework/Headers',
        ],
        'link_settings': {
          'libraries': [
            '-framework Carbon',
            '-framework CoreFoundation',
            '-framework ApplicationServices',
            '-framework OpenGL'
          ]
        }
      }],
      
      ['OS == "linux"', {
        'link_settings': {
          'libraries': [
            '-lX11',
            '-lXtst'
          ]
        },
        
        'sources': [
          'src/xdisplay.c'
        ]
      }],

      ["OS=='win'", {
        'defines': ['IS_WINDOWS']
      }],

      ['robotjs_enable_png==1', {
        'defines': ['ROBOTJS_HAS_PNG=1'],
        'sources': [
          'src/png_io.c'
        ],
        'conditions': [
          ['OS == "mac"', {
            'include_dirs': [
              '<!@(sh -c "pkg-config --cflags-only-I libpng | sed s/-I//g")'
            ],
            'link_settings': {
              'libraries': [
                '<!@(pkg-config --libs libpng)',
                '-lz'
              ]
            }
          }],
          ['OS == "linux"', {
            'include_dirs': [
              '<!@(sh -c "pkg-config --cflags-only-I libpng | sed s/-I//g")'
            ],
            'link_settings': {
              'libraries': [
                '<!@(pkg-config --libs libpng)',
                '-lz'
              ]
            }
          }]
        ]
      }, {
        'defines': ['ROBOTJS_HAS_PNG=0']
      }]
    ],
    
    'sources': [
      'src/robotjs.cc',
      'src/io.c',
      'src/bmp_io.c',
      'src/MMPointArray.c',
      'src/deadbeef_rand.c',
      'src/UTHashTable.c',
      'src/bitmap_find.c',
      'src/color_find.c',
      'src/mouse.c',
      'src/keypress.c',
      'src/keycode.c',
      'src/screen.c',
      'src/screengrab.c',
      'src/snprintf.c',
      'src/MMBitmap.c'
    ]
  }]
}
