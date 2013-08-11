ifeq (${PREFIX},)
	PREFIX=/usr
endif
sharedir = $(DESTDIR)$(PREFIX)/share
libexecdir = $(DESTDIR)$(PREFIX)/lib

all: ibus-engine-rime

ibus-engine-rime:
	mkdir -p build
	(cd build; cmake -DCMAKE_BUILD_TYPE=Release .. && make)
	@echo ':)'

ibus-engine-rime-static:
	mkdir -p build
	(cd build; cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON .. && make)
	@echo ':)'

install:
	mkdir -p $(sharedir)/ibus/component
	cp rime.xml $(sharedir)/ibus/component/
	mkdir -p $(libexecdir)/ibus-rime
	cp -f build/ibus-engine-rime $(libexecdir)/ibus-rime/
	mkdir -p $(sharedir)/ibus-rime
	mkdir -p $(sharedir)/ibus-rime/icons
	cp -f rime.png $(sharedir)/ibus-rime/icons/

uninstall:
	rm $(sharedir)/ibus/component/rime.xml
	rm -R $(sharedir)/ibus-rime
	rm -R $(libexecdir)/ibus-rime

clean:
	if  [ -e build ]; then rm -R build; fi
