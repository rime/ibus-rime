#!/bin/bash

if [ ! -e librime -a -e ../librime ]; then ln -s ../librime; fi
if [ ! -e brise -a -e ../brise ]; then ln -s ../brise; fi

if [ ! -e lib ]; then ln -s librime/thirdparty/lib; fi
if [ ! -e lib/libyaml-cpp.a ]; then (cd librime; make thirdparty) || exit 1; fi

(cd librime; make librime-static && sudo make -C build-static install) || exit 1

(cd brise; make && sudo make install) || exit 1

make clean && make ibus-engine-rime-static && sudo make install || exit 1

if [ "$1" == '--restart' ]; then
  ibus-daemon -drx
fi
