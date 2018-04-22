#!/bin/bash

git submodule update --init

(cd librime; make && sudo make install) || exit 1
(cd plum; make && sudo make install) || exit 1

make clean && make && sudo make install || exit 1

if [ "$1" == '--restart' ]; then
  ibus-daemon -drx
fi
