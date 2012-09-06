(cd ../librime; make && sudo make install) || exit 1
(cd ../brise; make && sudo make install) || exit 1
make clean && make && sudo make install && ibus-daemon -drx
