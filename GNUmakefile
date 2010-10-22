.PHONY: normal debug clean

normal:
	make -C libs/appengine $@
	make -C libs/exiftags $@
	make -C libs/flex $@
	make -C libs/fortify $@
	make -C libs/jpeg $@
	make -C libs/md5 $@
	make -C libs/png $@
	make -C libs/zlib $@
	make -C apps/!PrivatEye $@

debug:
	make -C libs/appengine $@
	make -C libs/exiftags $@
	make -C libs/flex $@
	make -C libs/fortify $@
	make -C libs/jpeg $@
	make -C libs/md5 $@
	make -C libs/png $@
	make -C libs/zlib $@
	make -C apps/!PrivatEye $@

clean:
	make -C libs/appengine $@
	make -C libs/exiftags $@
	make -C libs/flex $@
	make -C libs/fortify $@
	make -C libs/jpeg $@
	make -C libs/md5 $@
	make -C libs/png $@
	make -C libs/zlib $@
	make -C apps/!PrivatEye $@

all:	normal debug
