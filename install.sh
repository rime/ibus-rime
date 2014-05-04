#!/bin/bash

if [ ! -e librime -a -e ../librime ]; then ln -s ../librime; fi
if [ ! -e brise -a -e ../brise ]; then ln -s ../brise; fi

(cd librime; make && sudo make install) || exit 1
(cd brise; make && sudo make install) || exit 1

make clean && make && sudo make install || exit 1

if [ "$1" == '--restart' ]; then
  ibus-daemon -drx
fi
