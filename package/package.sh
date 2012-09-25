#!/bin/bash
if [ -z "$1" -o -z "$2" ]; then
  echo "usage: `basename $0` {rime-version} {brise-version}"
  exit 1
fi
rime_version=$1
brise_version=$2
mkdir -p temp
cd temp
for x in brise librime ibus-rime; do
  if [ -d $x ]; then rm -Rf $x; fi
  git clone ../../../$x && rm -Rf $x/.git
done
tar czvf ../brise-$brise_version.tar.gz brise/
tar czvf ../librime-$rime_version.tar.gz librime/
tar czvf ../ibus-rime-$rime_version.tar.gz ibus-rime/
cd ..
rm -Rf temp
echo 
echo 'built:'
ls *-$brise_version.tar.gz *-$rime_version.tar.gz
