{ nixroot  ? (import <nixpkgs> {}) }:
let inherit (nixroot) stdenv pkgs lib
    chromaprint fftw flac libid3tag libmad libopus libshout libsndfile lilv 
    libusb1 libvorbis libebur128 pkgconfig portaudio portmidi protobuf qt5 glib
    rubberband scons sqlite taglib soundtouch vamp opusfile hidapi upower ccache git
    libGLU x11 lame lv2 makeWrapper
    clang-tools
    fetchurl
    gdb
    python3;

  git-clang-format = stdenv.mkDerivation {
    name = "git-clang-format";
    version = "2019-06-21";
    src = fetchurl {
      url = "https://raw.githubusercontent.com/llvm-mirror/clang/2bb8e0fe002e8ffaa9ce5fa58034453c94c7e208/tools/clang-format/git-clang-format";
      sha256 = "1kby36i80js6rwi11v3ny4bqsi6i44b9yzs23pdcn9wswffx1nlf";
      executable = true;
    };
    nativeBuildInputs = [
      makeWrapper
    ];
    buildInputs = [
      clang-tools
      python3
    ];
    unpackPhase = ":";
    installPhase = ''
      mkdir -p $out/opt $out/bin
      cp $src $out/opt/git-clang-format
      makeWrapper $out/opt/git-clang-format $out/bin/git-clang-format \
        --add-flags --binary \
        --add-flags ${clang-tools}/bin/clang-format
    '';
  };

  shell-build = nixroot.writeShellScriptBin "build" ''
    scons \
      -j$NIX_BUILD_CORES \
      $sconsFlags "prefix=$HOME/mixxx" "$@";
  '';

  shell-run = nixroot.writeShellScriptBin "run" ''
      BUILDDIR=$(ls -1 -d -t lin64_build lin_build | head -1)
      $BUILDDIR/mixxx --settingsPath ./devsettings/ --resourcePath ./res "$@"
  '';

  shell-debug = nixroot.writeShellScriptBin "debug" ''
      BUILDDIR=$(ls -1 -d -t lin64_build lin_build | head -1)
      gdb --args $BUILDDIR/mixxx --settingsPath ./devsettings/ --resourcePath ./res "$@"
  '';

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

    echo -e "Mixxx development shell. Available commands:\n"
    echo " build - compiles Mixxx"
    echo " run - runs Mixxx with development settings"
    echo " debug - runs Mixxx inside gdb"
      '';

  src = builtins.filterSource
     (path: type: ! builtins.any (x: x == baseNameOf path) [ ".git" "cache" "lin64_build" "lin_build" "debian" ])
     ./.;

  nativeBuildInputs = [
    gdb
    git-clang-format
    shell-build shell-run shell-debug
  ];

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
