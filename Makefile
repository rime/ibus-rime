ifeq (${PREFIX},)
	PREFIX=/usr
endif
sharedir = $(PREFIX)/share
libexecdir = $(PREFIX)/lib

ifeq (${builddir},)
	builddir=build
endif

all: ibus-engine-rime

ibus-engine-rime:
	mkdir -p $(builddir)
	(cd $(builddir); cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_DATADIR=$(sharedir) -DCMAKE_INSTALL_LIBEXECDIR=$(libexecdir) .. && make)
	@echo ':)'

ibus-engine-rime-static:
	mkdir -p $(builddir)
	(cd $(builddir); cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_DATADIR=$(sharedir) -DCMAKE_INSTALL_LIBEXECDIR=$(libexecdir) -DBUILD_STATIC=ON .. && make)
	@echo ':)'

install:
	(cd $(builddir); make install)

uninstall:
	rm $(sharedir)/ibus/component/rime.xml
	rm -R $(sharedir)/ibus-rime
	rm -R $(libexecdir)/ibus-rime
	rm $(sharedir)/rime-data/ibus_rime.yaml

clean:
	if  [ -e $(builddir) ]; then rm -R $(builddir); fi
