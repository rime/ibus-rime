#!/bin/sh

POT="po/message.pot"
PO_FILES="po/*.po"

PACKAGE_NAME="ibus-rime"
BUGS_ADDRESS="https://github.com/rime/ibus-rime/issues"
VERSION="1.6.1"

xgettext \
    --package-name="$PACKAGE_NAME" \
    --package-version="$VERSION" \
    --msgid-bugs-address="$BUGS_ADDRESS" \
    --language=C \
    --keyword=_ \
    --output="$POT" \
    *.c

for po in $PO_FILES; do
    msgmerge --update "$po" "$POT"
done