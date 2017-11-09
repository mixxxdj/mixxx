{ nixroot  ? (import <nixpkgs> {}) }:
let inherit (nixroot) stdenv
    chromaprint fftw flac libid3tag libmad libopus libshout libsndfile
    libusb1 libvorbis libebur128 pkgconfig portaudio portmidi protobuf qt4 qt5
    rubberband scons sqlite taglib soundtouch vamp opusfile hidapi upower ccache git;
  
in stdenv.mkDerivation rec {
  name = "mixxx-${version}";
  version = "2.1.0-dev";

  shellHook =  ''
    export out=lin_build
    mkdir -p $out
    export CC="ccache $CC"
    export CXX="ccache $CXX"
    export out="local_build"

    build() {
      scons $sconsFlags prefix=~/mixxx $@
    }

    run() {
      ./lin*_build/mixxx --settingsPath ./devsettings/ --resourcePath ./res $@
    }

    echo -e "mixxx development shell. available commands:\n"
    echo " build - compiles mixxx"
    echo " run - runs mixxx with development settings"
      '';
  src = ./src;
  
  buildInputs = [
    chromaprint fftw flac libid3tag libmad libopus libshout libsndfile
    libusb1 libvorbis libebur128 pkgconfig portaudio portmidi protobuf qt4 qt5.full
    rubberband scons sqlite taglib soundtouch vamp.vampSDK opusfile upower hidapi ccache git
  ];

  sconsFlags = [
    "build=debug"
    "qtdir=${qt4}"
  ];

  buildPhase = ''
    runHook preBuild;
    mkdir -p "$out";
    scons \
      -j$NIX_BUILD_CORES -l$NIX_BUILD_CORES \
      $sconsFlags "prefix=$out";
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall
    scons $sconsFlags "prefix=$out" install
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
