{
  "targets": [
    {
      "target_name": "robotjs",
      "conditions": [
	      ['OS == "mac"', {
		      'include_dirs': [
		          'System/Library/Frameworks/CoreFoundation.Framework/Headers',
		          'System/Library/Frameworks/Carbon.Framework/Headers',
		        ],
		      "link_settings": {
	                            "libraries": [
	                                "-framework Carbon",
	                                "-framework CoreFoundation"
	                            ]
	                        }
	            }
	          ]
	        ],
	      "sources": ["src/robotjs.cc", "src/deadbeef_rand.c", "src/mouse.c", "src/screen.c"]
    }
  ]
}
