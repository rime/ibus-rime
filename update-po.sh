#!/bin/sh

if [ -z "$1" ]; then
    printf "Enter version: "
    read VERSION
else
    VERSION="$1"
fi

if [ -z "$VERSION" ]; then
    echo "Version cannot be empty"
    exit 1
fi

POT="po/message.pot"
PO_FILES="po/*.po"

PACKAGE_NAME="ibus-rime"
BUGS_ADDRESS="https://github.com/rime/ibus-rime/issues"

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