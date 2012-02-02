sharedir = /usr/share
libexecdir = /usr/lib

all: ibus-engine-rime ibus-rime-data
	@echo ':)'

librime:
	(cd ../librime; ./build.sh)

ibus-engine-rime: librime
	if [ ! -e cmake ]; then ln -s ../librime/cmake; fi
	mkdir -p build
	(cd build; cmake .. && 	make)

ibus-rime-data:
	mkdir -p data
	cp ../brise/default.yaml data/
	cp ../brise/essay.kct data/
	cp ../brise/preset/*.yaml data/

install:
	cp rime.xml $(sharedir)/ibus/component/
	mkdir -p $(libexecdir)/ibus-rime
	cp -f build/ibus-engine-rime $(libexecdir)/ibus-rime/
	mkdir -p $(sharedir)/ibus-rime
	cp -rf data/* $(sharedir)/ibus-rime/
	mkdir -p $(sharedir)/ibus-rime/icons
	cp -f zhung.png $(sharedir)/ibus-rime/icons/

uninstall:
	rm $(sharedir)/ibus/component/rime.xml
	rm -R $(sharedir)/ibus-rime
	rm -R $(libexecdir)/ibus-rime

clean:
	rm -R build
