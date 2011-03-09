.PHONY: normal debug clean

normal:
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

debug:
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

clean:
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

all:	normal debug
