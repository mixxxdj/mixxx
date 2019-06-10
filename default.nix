{ nixroot  ? (import <nixpkgs> {}) }:
let inherit (nixroot) stdenv pkgs lib
    chromaprint fftw flac libid3tag libmad libopus libshout libsndfile lilv 
    libusb1 libvorbis libebur128 pkgconfig portaudio portmidi protobuf qt5 glib
    rubberband scons sqlite taglib soundtouch vamp opusfile hidapi upower ccache git
    libGLU x11 lame lv2 makeWrapper;

in stdenv.mkDerivation rec {
  name = "mixxx-${version}";
  # reading the version from git output is very hard to do without wasting lots of diskspace and runtime
  # reading version file is easy
  version = lib.strings.removeSuffix "\"\n" (
              lib.strings.removePrefix "#define MIXXX_VERSION \"" (
                builtins.readFile ./src/_version.h ));

  shellHook =  ''
    export CC="ccache gcc"
    export CXX="ccache g++"

    build() {
      scons $sconsFlags prefix=~/mixxx $@
    }

    run() {
      BUILDDIR=$(ls -1 -d -t lin64_build lin_build | head -1)
      $BUILDDIR/mixxx --settingsPath ./devsettings/ --resourcePath ./res $@
    }

    echo -e "mixxx development shell. available commands:\n"
    echo " build - compiles mixxx"
    echo " run - runs mixxx with development settings"
      '';

  src = builtins.filterSource
     (path: type: ! builtins.any (x: x == baseNameOf path) [ ".git" "cache" "lin64_build" "lin_build" "debian" ])
     ./.;
  
  buildInputs = [
    chromaprint fftw flac libid3tag libmad libopus libshout libsndfile
    libusb1 libvorbis libebur128 pkgconfig portaudio portmidi protobuf qt5.full
    rubberband scons sqlite taglib soundtouch vamp.vampSDK opusfile upower hidapi
    ccache git glib x11 libGLU lilv lame lv2 makeWrapper qt5.qtbase
  ];

  sconsFlags = [
    "build=debug"
    "qtdir=${qt5.full}"
  ];

  buildPhase = ''
    runHook preBuild;
    mkdir -p "$out";
    scons \
      -j$NIX_BUILD_CORES \
      $sconsFlags "prefix=$out";
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall
    scons $sconsFlags "prefix=$out" install
    wrapProgram $out/bin/mixxx --suffix QT_PLUGIN_PATH : ${qt5.qtbase}/${qt5.qtbase.qtPluginPrefix} --set QTDIR ${qt5.full}
    runHook postInstall
  '';

  meta = with nixroot.stdenv.lib; {
    homepage = https://mixxx.org;
    description = "Digital DJ mixing software";
    license = licenses.gpl2Plus;
    maintainers = [ maintainers.aszlig maintainers.goibhniu ];
    platforms = platforms.linux;
  };
}
