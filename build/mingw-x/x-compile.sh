#!/bin/sh
# Cross compile script

# export QTDIR=/media/disk-1/Qt/QtCreator/qt # edit this line and uncomment to avoid being asked.

if [ -z "$QTDIR" ]; then
  read -p "Enter the mountpoint/path to your Windows QTDIR: " QTDIR
  export QTDIR=$QTDIR
fi

cd `dirname $0`
cd ../..
qmake -spec "${PWD}/build/mingw-x/mkspecs/i586-mingw32msvc-g++" mixxx.pro && make
