PREFIX = /usr/local
#PREFIX = /mingw

include build_rule.mk

CONFIG_FILE = urg3d-config
INCLUDE_DIR = $(PREFIX)/include/
URGLIB_STATIC = liburg3d.a
URGLIB_SHARED = $(shell if test `echo $(OS) | grep Windows`; then echo "urg3d.dll"; else echo "liburg3d.so"; fi)
S_PREFIX = $(shell echo "$(PREFIX)" | sed "s/\//\\\\\\\\\//g")
S_LIBS = $(shell if test `echo $(OS) | grep Windows`; then echo "-lwsock32 -lsetupapi"; else if test `echo $(OS) | grep Mac`; then echo ""; else "-lrt"; fi)
all : $(CONFIG_FILE)
	cd src/ && $(MAKE)
	cd samples/ && $(MAKE)

clean :
	$(RM) $(CONFIG_FILE)
	cd src/ && $(MAKE) clean
	cd samples/ && $(MAKE) clean

install : all
	install -d $(INCLUDE_DIR)
	install -m 644 include/*.h $(INCLUDE_DIR)
	install -d $(PREFIX)/lib/
	install -m 644 src/$(URGLIB_STATIC) $(PREFIX)/lib/
	install -m 644 src/$(URGLIB_SHARED) $(PREFIX)/lib/
	install -d $(PREFIX)/bin/
	install -m 755 $(CONFIG_FILE) $(PREFIX)/bin/

uninstall :
	$(RM) -r $(INCLUDE_DIR)
	$(RM) $(PREFIX)/lib/$(URGLIB_STATIC)
	$(RM) $(PREFIX)/lib/$(URGLIB_SHARED)
	$(RM) $(PREFIX)/bin/$(CONFIG_FILE)

$(CONFIG_FILE) : $(CONFIG_FILE).in Makefile
	cat $(CONFIG_FILE).in | sed "s/PREFIX/$(S_PREFIX)/g" | sed "s/LIBS/$(S_LIBS)/g" > $(CONFIG_FILE)
