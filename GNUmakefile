.PHONY: normal debug all utils clean

normal:	utils
	$(MAKE) -C libs/appengine $@
	$(MAKE) -C libs/exiftags $@
	$(MAKE) -C libs/flex $@
	$(MAKE) -C libs/fortify $@
	$(MAKE) -C libs/jpeg $@
	$(MAKE) -C libs/md5 $@
	$(MAKE) -C libs/png $@
	$(MAKE) -C libs/zlib $@
	$(MAKE) -C apps/!PrivatEye $@
	$(MAKE) -C apps/!TagCloud $@

debug:	utils
	$(MAKE) -C libs/appengine $@
	$(MAKE) -C libs/exiftags $@
	$(MAKE) -C libs/flex $@
	$(MAKE) -C libs/fortify $@
	$(MAKE) -C libs/jpeg $@
	$(MAKE) -C libs/md5 $@
	$(MAKE) -C libs/png $@
	$(MAKE) -C libs/zlib $@
	$(MAKE) -C apps/!PrivatEye $@
	$(MAKE) -C apps/!TagCloud $@

all:	normal debug

utils:
	$(MAKE) -C utils/templheader normal

clean:
	$(MAKE) -C utils/templheader $@
	$(MAKE) -C libs/appengine $@
	$(MAKE) -C libs/exiftags $@
	$(MAKE) -C libs/flex $@
	$(MAKE) -C libs/fortify $@
	$(MAKE) -C libs/jpeg $@
	$(MAKE) -C libs/md5 $@
	$(MAKE) -C libs/png $@
	$(MAKE) -C libs/zlib $@
	$(MAKE) -C apps/!PrivatEye $@
	$(MAKE) -C apps/!TagCloud $@
