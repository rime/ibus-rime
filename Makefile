src = rime_main.c rime_engine.c
sharedir = /usr/share
libexecdir = /usr/lib

all: ibus-engine-rime
	@echo ':)'

ibus-engine-rime: $(src)
	g++ -o ibus-engine-rime $(src) `pkg-config --cflags --libs ibus-1.0` -I../librime/include -L../librime/build/lib -lrime -lyaml-cpp -lboost_filesystem -lboost_signals -lboost_system

install: ibus-engine-rime
	cp rime.xml $(sharedir)/ibus/component/
	mkdir -p $(libexecdir)/ibus-rime
	cp -f ibus-engine-rime $(libexecdir)/ibus-rime/
	mkdir -p $(sharedir)/ibus-rime
	mkdir -p $(sharedir)/ibus-rime/icons
	cp -f zhung.png $(sharedir)/ibus-rime/icons/

uninstall:
	rm $(sharedir)/ibus/component/rime.xml
	rm -R $(sharedir)/ibus-rime
	rm -R $(libexecdir)/ibus-rime
