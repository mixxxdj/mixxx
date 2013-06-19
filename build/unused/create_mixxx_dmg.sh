#!/bin/bash
MIXXX_OLD_PATH=../MacOS/mixxx
MIXXX_PATH=../MacOS/Mixxx
VOL_NAME="Mixxx"
TMP_DMG_DIR="/tmp/mixxx_dmg"
ARCH=macintel
VERSION=`grep VERSION src/defs.h | cut -d" " -f 3 | tr -d \"`
DMG_PATH="./mixxx-$VERSION-$ARCH"
DMG_ICON="src/osx/VolumeIcon.icns"
QT4_PATH="/usr/local/Trolltech/Qt-4.3.2"  # Don't use a trailing slash here (important)

if [ ! -d Mixxx.app ]
then
    echo "Error: Mixxx.app bundle doesn't exist. Run \"scons\" to create it."
    exit
fi

cd Mixxx.app
cd Contents
mkdir Frameworks
cd Frameworks
echo "Copying libraries..."
cp $QT4_PATH/lib/libQtGui.4.dylib .
cp $QT4_PATH/lib/libQtCore.4.dylib .
cp $QT4_PATH/lib/libQtOpenGL.4.dylib .
cp $QT4_PATH/lib/libQtXml.4.dylib .
cp $QT4_PATH/lib/libQt3Support.4.dylib .
cp $QT4_PATH/lib/libQtNetwork.4.dylib .
cp $QT4_PATH/lib/libQtSql.4.dylib .
cp /usr/local/lib/libportaudio.2.dylib .
cp /usr/local/lib/libmad.0.dylib .
cp /opt/local/lib/libid3tag.0.dylib .
cp /usr/local/lib/libvorbisfile.3.dylib .
cp /usr/local/lib/libvorbis.0.dylib .
cp /usr/local/lib/libogg.0.dylib .
cp /usr/local/lib/libsndfile.1.dylib .
cp /usr/local/lib/libFLAC.7.dylib .

echo "Changing library ids..."
install_name_tool -id @executable_path/../Frameworks/libQtGui.4.dylib libQtGui.4.dylib
install_name_tool -id @executable_path/../Frameworks/libQtCore.4.dylib libQtCore.4.dylib
install_name_tool -id @executable_path/../Frameworks/libQtOpenGL.4.dylib libQtOpenGL.4.dylib
install_name_tool -id @executable_path/../Frameworks/libQt3Support.4.dylib libQt3Support.4.dylib
install_name_tool -id @executable_path/../Frameworks/libQtXml.4.dylib libQtXml.4.dylib
install_name_tool -id @executable_path/../Frameworks/libQtNetwork.4.dylib libQtNetwork.4.dylib
install_name_tool -id @executable_path/../Frameworks/libQtSql.4.dylib libQtSql.4.dylib
install_name_tool -id @executable_path/../Frameworks/libportaudio.2.dylib libportaudio.2.dylib
install_name_tool -id @executable_path/../Frameworks/libmad.0.dylib libmad.0.dylib
install_name_tool -id @executable_path/../Frameworks/libid3tag.0.dylib libid3tag.0.dylib
install_name_tool -id @executable_path/../Frameworks/libvorbisfile.3.dylib libvorbisfile.3.dylib
install_name_tool -id @executable_path/../Frameworks/libvorbis.0.dylib libvorbis.0.dylib
install_name_tool -id @executable_path/../Frameworks/libogg.0.dylib libogg.0.dylib
install_name_tool -id @executable_path/../Frameworks/libsndfile.1.dylib libsndfile.1.dylib
install_name_tool -id @executable_path/../Frameworks/libFLAC.7.dylib libFLAC.7.dylib

echo "Renaming mixxx binary to capitalised Mixxx"
mv $MIXXX_OLD_PATH $MIXXX_PATH

echo "Changing library ids in the Mixxx binary..."
install_name_tool -change $QT4_PATH/lib/libQtGui.4.dylib @executable_path/../Frameworks/libQtGui.4.dylib $MIXXX_PATH
install_name_tool -change $QT4_PATH/lib/libQtCore.4.dylib @executable_path/../Frameworks/libQtCore.4.dylib $MIXXX_PATH
install_name_tool -change $QT4_PATH/lib/libQtOpenGL.4.dylib @executable_path/../Frameworks/libQtOpenGL.4.dylib $MIXXX_PATH
install_name_tool -change $QT4_PATH/lib/libQt3Support.4.dylib @executable_path/../Frameworks/libQt3Support.4.dylib $MIXXX_PATH
install_name_tool -change $QT4_PATH/lib/libQtXml.4.dylib @executable_path/../Frameworks/libQtXml.4.dylib $MIXXX_PATH
install_name_tool -change $QT4_PATH/lib/libQtNetwork.4.dylib @executable_path/../Frameworks/libQtNetwork.4.dylib $MIXXX_PATH
install_name_tool -change $QT4_PATH/lib/libQtSql.4.dylib @executable_path/../Frameworks/libQtSql.4.dylib $MIXXX_PATH
install_name_tool -change /usr/local/lib/libportaudio.2.dylib @executable_path/../Frameworks/libportaudio.2.dylib $MIXXX_PATH
install_name_tool -change /usr/local/lib/libmad.0.dylib @executable_path/../Frameworks/libmad.0.dylib $MIXXX_PATH
install_name_tool -change /opt/local/lib/libid3tag.0.dylib @executable_path/../Frameworks/libid3tag.0.dylib $MIXXX_PATH # *** /opt/local/lib for this one!
install_name_tool -change /usr/local/lib/libvorbisfile.3.dylib @executable_path/../Frameworks/libvorbisfile.3.dylib $MIXXX_PATH
install_name_tool -change /usr/local/lib/libvorbis.0.dylib @executable_path/../Frameworks/libvorbis.0.dylib $MIXXX_PATH
install_name_tool -change /usr/local/lib/libogg.0.dylib @executable_path/../Frameworks/libogg.0.dylib $MIXXX_PATH
install_name_tool -change /usr/local/lib/libsndfile.1.dylib @executable_path/../Frameworks/libsndfile.1.dylib $MIXXX_PATH
install_name_tool -change /usr/local/lib/libFLAC.7.dylib @executable_path/../Frameworks/libFLAC.7.dylib $MIXXX_PATH

echo "Changing library ids within the QT4 libs"
# These are intra-QT4 dependencies
install_name_tool -change $QT4_PATH/lib/libQtCore.4.dylib @executable_path/../Frameworks/libQtCore.4.dylib libQtGui.4.dylib
install_name_tool -change $QT4_PATH/lib/libQtCore.4.dylib @executable_path/../Frameworks/libQtCore.4.dylib libQtOpenGL.4.dylib
install_name_tool -change $QT4_PATH/lib/libQtGui.4.dylib @executable_path/../Frameworks/libQtGui.4.dylib libQtOpenGL.4.dylib
install_name_tool -change $QT4_PATH/lib/libQtCore.4.dylib @executable_path/../Frameworks/libQtCore.4.dylib libQtSql.4.dylib
install_name_tool -change $QT4_PATH/lib/libQtCore.4.dylib @executable_path/../Frameworks/libQtCore.4.dylib libQtNetwork.4.dylib
install_name_tool -change $QT4_PATH/lib/libQtCore.4.dylib @executable_path/../Frameworks/libQtCore.4.dylib libQtXml.4.dylib
install_name_tool -change $QT4_PATH/lib/libQtGui.4.dylib @executable_path/../Frameworks/libQtGui.4.dylib libQt3Support.4.dylib
install_name_tool -change $QT4_PATH/lib/libQtCore.4.dylib @executable_path/../Frameworks/libQtCore.4.dylib libQt3Support.4.dylib
install_name_tool -change $QT4_PATH/lib/libQtSql.4.dylib @executable_path/../Frameworks/libQtSql.4.dylib libQt3Support.4.dylib
install_name_tool -change $QT4_PATH/lib/libQtXml.4.dylib @executable_path/../Frameworks/libQtXml.4.dylib libQt3Support.4.dylib
install_name_tool -change $QT4_PATH/lib/libQtNetwork.4.dylib @executable_path/../Frameworks/libQtNetwork.4.dylib libQt3Support.4.dylib


echo "Changing library ids within Vorbis lib"
# These are intra-ogg/vorbis dependencies (don't ask me how I figure this stuff out)
install_name_tool -change /opt/local/lib/libogg.0.dylib @executable_path/../Frameworks/libogg.0.dylib libvorbis.0.dylib
install_name_tool -change /opt/local/lib/libogg.0.dylib @executable_path/../Frameworks/libogg.0.dylib libvorbisfile.3.dylib
install_name_tool -change /usr/local/lib/libvorbis.0.dylib @executable_path/../Frameworks/libvorbis.0.dylib libvorbisfile.3.dylib # /usr for this one and /opt for the libogg ones... careful... (libogg is from MacPorts?)

echo "Changing library ids within libsndfile"
install_name_tool -change /usr/local/lib/libFLAC.7.dylib @executable_path/../Frameworks/libFLAC.7.dylib libsndfile.1.dylib

echo "Stripping debugging symbols in libraries"
for lib in `ls`;
do
    echo "Stripping $lib"
    strip -S $lib                      # Strip all the libraries
done

strip $MIXXX_PATH                # Strip the mixxx binary

echo "Copying QT4 imageformat plugins"
cd ..
mkdir plugins
mkdir plugins/imageformats
mkdir plugins/iconengines
cd plugins/imageformats

for lib in libqgif.dylib libqjpeg.dylib libqsvg.dylib; #Left out libqmng and libqtiff to save space.
do
    cp $QT4_PATH/plugins/imageformats/$lib .

    echo "Changing library ids within QT4 imageformat plugin: $lib"
    install_name_tool -id @executable_path/../Frameworks/$lib $lib
    install_name_tool -change $QT4_PATH/lib/libQtCore.4.dylib @executable_path/../Frameworks/libQtCore.4.dylib $lib
    install_name_tool -change $QT4_PATH/lib/libQtGui.4.dylib @executable_path/../Frameworks/libQtGui.4.dylib $lib
    install_name_tool -change $QT4_PATH/lib/libQtSql.4.dylib @executable_path/../Frameworks/libQtSql.4.dylib $lib
    install_name_tool -change $QT4_PATH/lib/libQtXml.4.dylib @executable_path/../Frameworks/libQtXml.4.dylib $lib
    install_name_tool -change $QT4_PATH/lib/libQtSvg.4.dylib @executable_path/../Frameworks/libQtSvg.4.dylib $lib
    install_name_tool -change $QT4_PATH/lib/libQtNetwork.4.dylib @executable_path/../Frameworks/libQtNetwork.4.dylib $lib
done
echo "Stripping debugging symbols from QT4 imageformat plugins"
strip *.dylib #Strip the imageformat plugins

#Rinse, repeat for the iconengine plugins
cd ..
cd iconengines

for lib in libqsvg.dylib;  #Yeah, theres a libqsvg.dylib iconengine and a libqsvg.dylib imageformat (and they're different)...
do
    cp $QT4_PATH/plugins/iconengines/$lib .

    echo "Changing library ids within QT4 iconegine plugin: $lib"
    install_name_tool -id @executable_path/../Frameworks/$lib $lib
    install_name_tool -change $QT4_PATH/lib/libQtCore.4.dylib @executable_path/../Frameworks/libQtCore.4.dylib $lib
    install_name_tool -change $QT4_PATH/lib/libQtGui.4.dylib @executable_path/../Frameworks/libQtGui.4.dylib $lib
    install_name_tool -change $QT4_PATH/lib/libQtSql.4.dylib @executable_path/../Frameworks/libQtSql.4.dylib $lib
    install_name_tool -change $QT4_PATH/lib/libQtXml.4.dylib @executable_path/../Frameworks/libQtXml.4.dylib $lib
    install_name_tool -change $QT4_PATH/lib/libQtSvg.4.dylib @executable_path/../Frameworks/libQtSvg.4.dylib $lib
    install_name_tool -change $QT4_PATH/lib/libQtNetwork.4.dylib @executable_path/../Frameworks/libQtNetwork.4.dylib $lib
done
echo "Stripping debugging symbols from QT4 iconengine plugins"
strip *.dylib #Strip the iconengine plugins
cd ..


cd ../../../
echo "Done creating Mixxx.app bundle!"

echo "Building DMG..."
rm -rf "$TMP_DMG_DIR"                               # Ditch old temp dmg directory
mkdir "$TMP_DMG_DIR"                                # Create a temp dmg directory
cp -r Mixxx.app "$TMP_DMG_DIR"                      # Copy Mixxx.app there
cp README "$TMP_DMG_DIR"                            # Copy the README too
cp LICENSE "$TMP_DMG_DIR"                           # ... and the LICENSE
cp Mixxx-Manual.pdf "$TMP_DMG_DIR"                  # ... and the manual
cp "$DMG_ICON" "$TMP_DMG_DIR/.VolumeIcon.icns"      # Copy the icon for the dmg
SetFile -a C "$TMP_DMG_DIR"                         # Set the folder's icon
# Might need to do the SetFile on the final .dmg instead...?
hdiutil create -srcfolder "$TMP_DMG_DIR" -format UDBZ -volname "$VOL_NAME" "$DMG_PATH"

