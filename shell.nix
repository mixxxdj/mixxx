{ pkgs ? (import <nixpkgs> {})
, lib ? pkgs.lib
, defaultLv2Plugins ? false
, lv2Plugins ? []
}:

let
  shell-configure = pkgs.writeShellScriptBin "configure" ''
    mkdir -p cbuild
    cd cbuild
    cmake .. "$@"
    cd ..
  '';

  shell-build = pkgs.writeShellScriptBin "build" ''
    if [ ! -d "cbuild" ]; then
      >&2 echo "First you have to run configure."
      exit 1
    fi
    cd cbuild
    cmake --build . --parallel $NIX_BUILD_CORES "$@"
    source ${pkgs.makeWrapper}/nix-support/setup-hook
    wrapProgram mixxx --prefix LV2_PATH : ${lib.makeSearchPath "lib/lv2" allLv2Plugins}
  '';

  shell-run = pkgs.writeShellScriptBin "run" ''
    if [ ! -f "cbuild/mixxx" ]; then
      >&2 echo "First you have to run build."
      exit 1
    fi
    cd cbuild
    ./mixxx --resourcePath res/ "$@"
  '';

  shell-debug = pkgs.writeShellScriptBin "debug" ''
    if [ ! -f "cbuild/mixxx" ]; then
      >&2 echo "First you have to run build."
      exit 1
    fi
    cd cbuild
    LV2_PATH=${lib.makeSearchPath "lib/lv2" allLv2Plugins} gdb --args ./.mixxx-wrapped --resourcePath res/ "$@"
  '';

  allLv2Plugins = lv2Plugins ++ (if defaultLv2Plugins then with pkgs; [
    artyFX
    infamousPlugins
    mod-distortion
    rkrlv2
    x42-plugins
    zam-plugins
  ] else []);

in pkgs.mkShell rec {
  buildInputs = with pkgs; [
    ccache
    chromaprint
    clang-tools
    cmake
    ffmpeg
    fftw
    flac
    gdb
    git
    glib
    hidapi
    lame
    libGLU
    libebur128
    libid3tag
    libmad
    libmodplug
    libopus
    libsForQt5
    libselinux
    libsepol
    libshout
    libsndfile
    libusb1
    libvorbis
    lilv
    lv2
    mp4v2
    opusfile
    pcre
    pkgconfig
    portaudio
    portmidi
    protobuf
    qt5
    rubberband
    soundtouch
    sqlite
    taglib
    upower
    utillinux
    vamp
    wavpack
    x11

    shell-configure
    shell-build
    shell-run
    shell-debug
    allLv2Plugins
  ];

  # SOURCE_DATE_EPOCH helps with python and pre-commit hook
  shellHook =  ''
    export PYTHONPATH=venv/lib/python3.7/site-packages/:$PYTHONPATH
    export SOURCE_DATE_EPOCH=315532800
    echo -e "Mixxx development shell. Available commands:\n"
    echo " configure - configures cmake (only has to run once)"
    echo " build - compiles Mixxx"
    echo " run - runs Mixxx with development settings"
    echo " debug - runs Mixxx inside gdb"
  '';
}
