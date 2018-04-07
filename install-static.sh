#!/bin/bash

if [ ! -e lib ]; then ln -s librime/thirdparty/lib; fi
(cd librime; make thirdparty) || exit 1

(cd librime; make librime-static && sudo make -C build-static install) || exit 1

(cd plum; make && sudo make install) || exit 1

make clean && make ibus-engine-rime-static && sudo make install || exit 1

if [ "$1" == '--restart' ]; then
  ibus-daemon -drx
fi
