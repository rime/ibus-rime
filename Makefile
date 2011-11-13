src = rime_main.cc rime_engine.cc
sharedir = /usr/share
libexecdir = /usr/lib

all: ibus-engine-rime
	@echo ':)'

ibus-engine-rime: $(src)
	if [ ! -e cmake ]; then ln -s ../librime/cmake; fi
	mkdir -p build
	(cd build; cmake .. && 	make)

install: ibus-engine-rime
	cp rime.xml $(sharedir)/ibus/component/
	mkdir -p $(libexecdir)/ibus-rime
	cp -f build/ibus-engine-rime $(libexecdir)/ibus-rime/
	mkdir -p $(sharedir)/ibus-rime
	mkdir -p $(sharedir)/ibus-rime/icons
	cp -f zhung.png $(sharedir)/ibus-rime/icons/

uninstall:
	rm $(sharedir)/ibus/component/rime.xml
	rm -R $(sharedir)/ibus-rime
	rm -R $(libexecdir)/ibus-rime

clean:
	rm -R build