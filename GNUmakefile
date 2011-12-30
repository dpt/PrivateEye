.PHONY: normal debug all utils clean

normal:	utils
	make -C libs/appengine $@
	make -C libs/exiftags $@
	make -C libs/flex $@
	make -C libs/fortify $@
	make -C libs/jpeg $@
	make -C libs/md5 $@
	make -C libs/png $@
	make -C libs/zlib $@
	make -C apps/!PrivatEye $@
	make -C apps/!TagCloud $@

debug:	utils
	make -C libs/appengine $@
	make -C libs/exiftags $@
	make -C libs/flex $@
	make -C libs/fortify $@
	make -C libs/jpeg $@
	make -C libs/md5 $@
	make -C libs/png $@
	make -C libs/zlib $@
	make -C apps/!PrivatEye $@
	make -C apps/!TagCloud $@

all:	normal debug

utils:
	make -C utils/templheader normal

clean:	utils
	make -C libs/appengine $@
	make -C libs/exiftags $@
	make -C libs/flex $@
	make -C libs/fortify $@
	make -C libs/jpeg $@
	make -C libs/md5 $@
	make -C libs/png $@
	make -C libs/zlib $@
	make -C apps/!PrivatEye $@
	make -C apps/!TagCloud $@
