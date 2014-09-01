{
  "targets": [
    {
      "target_name": "autojs",
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
	      "sources": ["src/autojs.cc", "src/deadbeef_rand.c", "src/mouse.c", "src/screen.c"]
    }
  ]
}
