#!/bin/bash

git submodule update --init

if [ ! -e lib ]; then ln -s librime/thirdparty/lib; fi
(cd librime; make thirdparty) || exit 1

(cd librime; make librime-static && sudo make -C build-static install) || exit 1

(cd plum; make && sudo make install) || exit 1

make builddir=build-static clean && \
make builddir=build-static ibus-engine-rime-static && \
sudo make builddir=build-static install || exit 1

if [ "$1" == '--restart' ]; then
  ibus-daemon -drx
fi
