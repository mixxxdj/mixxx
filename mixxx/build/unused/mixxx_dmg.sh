#!/bin/bash

#XXX this script assumes you have installed Qt with the default options (to /Developer and /Library/Frameworks). If you have not done this (e.g. have built with --no-framework which puts Qt in /usr/local/Trolltech) then stuff will need changing

#XXX
#todo: separate the 'make app' part and the 'make dmg' part.
#todo: then rewrite this in scons? if it becomes part of the rest of SConstruct then we can be smart about what libs to copy in (i.e. not copy QtScript if we don't have scripting on)

#notes to the sconsful-future:
#framework_path_template = "/Library/Frameworks/$1.framework/$1"
#dylib_path_template = "/usr/local/lib/lib$1.dylib"
#


#XXX libraries with similar names (e.g. libvorbis and libvorbisfile) can cause problems for the otool parsing magic.


WRKDIR=`pwd` #this should be the 'mixxx/', the root of the svn branch
MIXXX_OLD_PATH=Contents/MacOS/mixxx #relative to the root of the bundle
MIXXX_PATH=Contents/MacOS/Mixxx
VOL_NAME="Mixxx"
TMP_DMG_DIR="/tmp/mixxx_dmg"
ARCH=macintel
VERSION=`grep VERSION src/defs.h | cut -d" " -f 3 | tr -d \"` #uuuh right. This should be pulled from wherever scons pulls it from. This leaves a ^M on the tail end of the output filename
DMG_PATH="./mixxx-$VERSION-$ARCH"
DMG_ICON="src/osx/VolumeIcon.icns"
#QT4_PATH="/usr/local/Trolltech/Qt-4.3.2"  # Don't use a trailing slash here (important)
QT4_PLUGINS="/Developer/Applications/Qt/plugins/"
#XXX we should really do this by like, declaring the frameworks and dylibs we're using at the top and then being smart about all this
#FRAMEWORKS are shared libraries with an Apple-esque naming convention. DYLIBS are classical .so object files.
#(ASSUMPTION: every dylibg is under /usr/local/lib. Anything under /usr/lib should be already on any systems we package for. The one exception to this might be things in /opt or /sw which darwinports and fink use :/... bah when we rewrite this for SCons we can make it smarter
FRAMEWORKS="QtCore QtGui QtOpenGL QtXml Qt3Support QtNetwork QtSvg QtSql QtScript" #XXX should only do this if mixxx was built with scripting... oh well, a TODO
DYLIBS="portaudio mad id3tag vorbis vorbisfile ogg sndfile FLAC"


##### FUNCTIONS ####




cp_framework() {
    cp /Library/Frameworks/$1.framework/$1 $1 #copy explicitly so that symlinks don't change the paths underneath us
}



cp_dylib() {
    cp /usr/local/lib/lib$1.dylib lib$1.dylib #copy explicitly so that symlinks don't change the paths underneath us
}



#OS X links libraries based on pseudo-paths that every object file carries (the list can be seen with otool -L)
#the main purpose of this file is to rewrite all these strings so that Mixxx uses the libraries in it's .app bundle, and thus can be put up for download by other computrons

#relink_lib is for changing a library's ID *within itself* to how our packaging names them
relink_lib() {
    echo "DEBUG: RELINK_LIB: $1"
    install_name_tool -id @executable_path/../Frameworks/$1 $1
}

#reref is for changing the tables of links in files to other files.
#eeep, this part is pretty hairy. It relies on some dirty shell magic to parse otool's output. Need to do this because install_name_tool needs the exact path Mixxx is referencing to do the updates
#XXX the ordering of arguments here is unconventional and confusing
reref() { #takes the path to the binary to change, and the shortname of the lib to change
    echo "DEBUG: REREF: $1 $2 $3"
    TAB=`printf "\t"` #because I can't write \t to get a tab in shell :(
    path=`otool -L $1 | tail +2 | cut -f 2 -d "$TAB" | cut -f 1 -d "(" | grep "$2\..*"` #SHELL_FUUUUUUU!!!!! (the last bit is important, it makes sure we only get the libs ending in the given name)
    if [ x"$path" = x ]; then
	#echo "$1 does not reference $2, skipping";
	echo -n;
    else
	#echo "DEBUG: " install_name_tool -change $path @executable_path/../Frameworks/$3 $1
	install_name_tool -change $path @executable_path/../Frameworks/$3 $1
    fi
}



framework_reref_mixxx() { #bad name!
    reref $MIXXX_PATH $1 $1
}

dylib_reref_mixxx() { #still a bad name!
    reref $MIXXX_PATH $1 lib$1.dylib
}



relink_dylib() {
    relink_lib "lib$1.dylib"
}

#takes $1 = old libpath (the one Mixxx was compiled against)
mixxx_framework() {
    install_name_tool -change /Library/Frameworks/$1.framework/$1 @executable_path/../Frameworks/$1 $MIXXX_PATH
}

mixxx_dylib() {
    install_name_tool -change /usr/local/lib/lib$1.dylib 
}


#patch up the references in a framework lib that depends on another framework lib
#read reref_framework A B as "A depends on B"
reref_framework() {
    
    reref $1 $2 $2
}

#XXX do we need to cover the cases of framework-referencing-dylib and vice versa?

#patch up the references in a .dylib that depends on another .dylib
reref_dylib() {
    reref lib$1.dylib $2 lib$2.dylib
}


##### MAIN #####

if [ ! -d Mixxx.app ]
then
    echo "Error: Mixxx.app bundle doesn't exist. Run \"scons\" to create it."
    exit
fi

cd Mixxx.app

echo -n "Renaming 'mixxx' binary to capitalised 'Mixxx' (workaround for bug on 10.5 (?))"
mv $MIXXX_OLD_PATH $MIXXX_PATH
echo "."


for i in $FRAMEWORKS; do
    framework_reref_mixxx $i
done

for i in $DYLIBS; do
    dylib_reref_mixxx $i
done

echo -n "Stripping Mixxx binary of debugging symbols"
strip $MIXXX_PATH
echo "."


#make the bundle's Frameworks dir
echo "DEBUG: cleaning Frameworks/"
rm -r Contents/Frameworks/
echo "Making Frameworks directory"
mkdir Contents/Frameworks/
cd Contents/Frameworks

#XXX todo: roll together all the processing of each lib into one library. We can speed things up with PARALLELLLLLISISISM too then, by just calling `process_framework X &`


#copy in all the libs we need and use otool to relink their paths so the final binary is 

echo "Copying libraries..."
for i in $FRAMEWORKS; do
    cp_framework $i
done
for i in $DYLIBS; do
    cp_dylib $i
done






echo "Changing library ids..."
for i in $FRAMEWORKS; do
    relink_lib $i;
done

for i in $DYLIBS; do
    relink_dylib $i;
done



echo "Changing library ids in the Mixxx binary..."








echo "Changing library ids within the QT4 libs"
# These are intra-QT4 dependencies (yay hacks)
#XXX this would be a good place to run qtdeploy maybe?
#XXX otool -L magic could be used to generate these Qt dependency lists
reref_framework QtGui QtCore
reref_framework QtOpenGL QtCore
reref_framework QtOpenGL QtGui
reref_framework QtSql QtCore
reref_framework QtNetwork QtCore
reref_framework QtXml QtCore
reref_framework QtSvg QtCore
reref_framework QtSvg QtGui
reref_framework Qt3Support QtGui
reref_framework Qt3Support QtCore
reref_framework Qt3Support QtSql
reref_framework Qt3Support QtXml
reref_framework Qt3Support QtNetwork
reref_framework QtScript QtCore


echo "Changing library ids within Vorbis lib"
#Actually, otool -L could be used to generate all of these dependency lists
# These are intra-ogg/vorbis dependencies (don't ask me how I figure this stuff out)
reref_dylib vorbis ogg
reref_dylib vorbisfile ogg
reref_dylib vorbisfile vorbis

echo "Changing library ids within libsndfile"
reref_dylib sndfile FLAC

echo "Stripping debugging symbols in libraries"
for lib in `ls`;
do
    echo "Stripping $lib"
    strip -S $lib                      # Strip all the libraries
done

cd ..

echo "Copying QT4 plugins"
mkdir plugins 
cd plugins

bundle_QT4_plugin() {
    plugin=$1
    libs=$2
    mkdir $plugin
    cd $plugin
    for lib in $libs; do
	lib=lib$lib.dylib #yay
	cp $QT4_PLUGINS/imageformats/$lib $lib

	echo "Changing library ids within QT4 $plugin plugin: $lib"
	relink_lib $lib
	#XXX this list really needs to be generated on the fly from otool ....
	reref $lib QtCore QtCore #not using reref_framework because I don't think it means the same thing. in reref $2 is a search string, $3 is a filename. reref_frameworks assumes both are identical.
	reref $lib QtGui QtGui
	reref $lib QtXml QtXml
	reref $lib QtSvg QtSvg
	reref $lib QtNetwork QtNetwork
    done
    echo "Stripping debugging symbols from QT4 $plugin plugins"
    strip *.dylib
    cd ..
}


bundle_QT4_plugin "imageformats" "qgif qjpeg qsvg"; #Left out libqmng and libqtiff to save space.
bundle_QT4_plugin "iconengines" "qsvg"; #Yeah, theres a libqsvg.dylib iconengine and a libqsvg.dylib imageformat (and they're different)...


cd $WRKDIR
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

