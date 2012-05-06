#!/bin/bash
if [ -z "$1" ]; then
  echo "usage: `basename $0` {version}"
  exit 1
fi
version=$1
mkdir -p temp
cd temp
for x in brise librime ibus-rime; do
  if [ -d $x ]; then rm -Rf $x; fi
  git clone ../../../$x && rm -Rf $x/.git
done
tar czvf ../librime-$version.tar.gz librime/
tar czvf ../ibus-rime-$version.tar.gz brise/ ibus-rime/
cd ..
rm -Rf temp
echo 
echo 'built:'
ls *-$version.tar.gz
