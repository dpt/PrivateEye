.PHONY:	build normal debug all utils clean

build:
	Dir utils.templheader
	Obey MakeMake
	Dir ^.^
	Dir libs.appengine
	Obey MakeMake
	Dir ^.^.libs.exiftags
	Obey MakeMake
	Dir ^.^.libs.jpeg
	CDir o
	CDir odf
	Dir ^.^.libs.md5
	Obey MakeMake
	Dir ^.^.libs.png
	CDir o
	CDir odf
	Dir ^.^.libs.zlib
	CDir o
	CDir odf
	Dir ^.^.apps.!PrivatEye
	Obey MakeMake
	Dir ^.^.apps.!TagCloud
	Obey MakeMake
	Dir ^.^

normal:	utils
	Dir libs.appengine
	amu $@
	Dir ^.^.libs.exiftags
	amu $@
	Dir ^.^.libs.fortify
	amu -f MakefileRO $@
	Dir ^.^.libs.jpeg
	amu -f MakefileRO libs
	Dir ^.^.libs.md5
	amu $@
	Dir ^.^.libs.png
	amu -f MakefileRO $@
	Dir ^.^.libs.zlib
	amu -f MakefileRO $@
	Dir ^.^.apps.!PrivatEye
	amu $@
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
	amu -f MakefileRO libs
	Dir ^.^.libs.md5
	amu $@
	Dir ^.^.libs.png
	amu -f MakefileRO $@
	Dir ^.^.libs.zlib
	amu -f MakefileRO $@
	Dir ^.^.apps.!PrivatEye
	amu $@
	Dir ^.^.apps.!TagCloud
	amu $@
	Dir ^.^

all:	build normal debug

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
	Dir ^.^.apps.!TagCloud
	-amu $@
	Dir ^.^
