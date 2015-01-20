{
  "targets": [
    {
      "target_name": "robotjs",
      "conditions": [
	      ['OS == "mac"', {
		      'include_dirs': [
		          'System/Library/Frameworks/CoreFoundation.Framework/Headers',
		          'System/Library/Frameworks/Carbon.Framework/Headers',
		          'System/Library/Frameworks/ApplicationServices.framework/Headers',
		          'System/Library/Frameworks/OpenGL.framework/Headers',
		        ],
		      "link_settings": {
	                            "libraries": [
	                                "-framework Carbon",
	                                "-framework CoreFoundation",
	                                "-framework ApplicationServices",
	                               	"-framework OpenGL"
	                            ]
	                        }
	            }
	          ]
	        ],
	      "sources": ["src/robotjs.cc", "src/deadbeef_rand.c", "src/mouse.c", "src/keypress.c" , "src/keycode.c"]
    }
  ]
}
