.PHONY:	normal debug all utils clean

normal:	utils
	Dir libs.appengine
	amu $@
	Dir ^.^.libs.exiftags
	amu $@
	Dir ^.^.libs.fortify
	amu -f MakefileRO $@
	Dir ^.^.libs.jpeg
	amu -f MakefileRO $@
	Dir ^.^.libs.md5
	amu $@
	Dir ^.^.libs.png
	amu -f MakefileRO $@
	Dir ^.^.libs.zlib
	amu -f MakefileRO $@
	Dir ^.^.apps.!PrivatEye
	amu $@
	Dir ^.^
	Dir ^.^.apps.!TagCloud
	amu $@
	Dir ^.^

debug:	utils
	Dir libs.appengine
	amu $@
	Dir ^.^.libs.exiftags
	amu $@
	Dir ^.^.libs.fortify
	amu -f MakefileRO $@
	Dir ^.^.libs.jpeg
	amu -f MakefileRO $@
	Dir ^.^.libs.md5
	amu $@
	Dir ^.^.libs.png
	amu -f MakefileRO $@
	Dir ^.^.libs.zlib
	amu -f MakefileRO $@
	Dir ^.^.apps.!PrivatEye
	amu $@
	Dir ^.^
	Dir ^.^.apps.!TagCloud
	amu $@
	Dir ^.^

all:	normal debug

utils:
	Dir utils.templheader
	amu normal
	Dir ^.^

clean:
	Dir utils.templheader
	-amu $@
	Dir ^.^
	Dir libs.appengine
	-amu $@
	Dir ^.^.libs.exiftags
	-amu $@
	Dir ^.^.libs.fortify
	-amu -f MakefileRO $@
	Dir ^.^.libs.jpeg
	-amu -f MakefileRO $@
	Dir ^.^.libs.md5
	-amu $@
	Dir ^.^.libs.png
	-amu -f MakefileRO $@
	Dir ^.^.libs.zlib
	-amu -f MakefileRO $@
	Dir ^.^.apps.!PrivatEye
	-amu $@
	Dir ^.^
