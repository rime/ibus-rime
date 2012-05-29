(cd ../librime; make && sudo make install) || exit 1
make clean && make && sudo make install && ibus-daemon -drx
