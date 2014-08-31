{
  "targets": [
    {
      "target_name": "autonode",
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
	      "sources": ["src/autonode.cc", "src/deadbeef_rand.c", "src/mouse.c", "src/screen.c"]
    }
  ]
}
