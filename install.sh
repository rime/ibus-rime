if [ -e build/ibus-engine-rime ]; then
  rm build/ibus-engine-rime
fi
make && sudo make install && ibus-daemon -drx
