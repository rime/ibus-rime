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

ibus-rime-data: ibus-engine-rime
	mkdir -p data
	cp ../brise/default.yaml data/
	cp ../brise/essay.kct data/
	cp ../brise/preset/*.yaml data/
	cp ../brise/supplement/*.yaml data/
	echo 'precompiling Rime schemas, patience...'
	(cd data; ../build/ibus-engine-rime --build)
	if [ -e data/rime.log ]; then rm data/rime.log; fi
	if [ -e data/installation.yaml ]; then rm data/installation.yaml; fi

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
	if  [ -e build ]; then rm -R build; fi
