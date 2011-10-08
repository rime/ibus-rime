all: ibus-engine-rime

ibus-engine-rime: rime_main.c rime_engine.c
	g++ -o ibus-engine-rime rime_main.c rime_engine.c `pkg-config --cflags --libs ibus-1.0` -I../librime/include -L../librime/build/lib -lrime -lyaml-cpp -lboost_filesystem -lboost_signals -lboost_system

install: ibus-engine-rime
	cp rime.xml /usr/share/ibus/component/
	mkdir -p /usr/lib/ibus-rime
	cp -f ibus-engine-rime /usr/lib/ibus-rime/
	mkdir -p /usr/share/ibus-rime
	cp -f data/* /usr/share/ibus-rime/
	mkdir -p /usr/share/ibus-rime/icons
	cp -f zhung.png /usr/share/ibus-rime/icons/
