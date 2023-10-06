{ pkgs  ? (import <nixpkgs> {})
, defaultLv2Plugins ? false
, lv2Plugins ? []
, releaseMode ? false
}:
let inherit (pkgs) stdenv lib
    clang-tools
    cmake
    fetchurl
    ccache
    gdb
    nix-gitignore
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
      pkgs.makeWrapper
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

  shell-configure = pkgs.writeShellScriptBin "configure" ''
    set -eux
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
    set -eux
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
    set -eux
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

  allLv2Plugins = lv2Plugins ++ (if defaultLv2Plugins then [
    pkgs.x42-plugins pkgs.zam-plugins pkgs.rkrlv2 pkgs.mod-distortion
    pkgs.infamousPlugins pkgs.artyFX
  ] else []);

in pkgs.qt6Packages.callPackage ({
    # buildInputs from <nixpkgs/pkgs/applications/audio/mixxx/default.nix>, Qt5->Qt6, removed qtscript and qtx11extras and added gtest, gbenchmark, qtdeclarative, qt5compat and microsoft_gsl for Qt6
    # as of https://github.com/NixOS/nixpkgs/commit/ab9cf80e428a8814cecb173efce840cc56b025eb
    chromaprint, faad2, ffmpeg, fftw, flac, glibcLocales, hidapi, lame, libebur128,
    libGLU, libid3tag, libkeyfinder, libmad, libmodplug, libopus, libsecret, libshout,
    libsndfile, libusb1, libvorbis, libxcb, lilv, lv2, mp4v2, opusfile, pcre, portaudio,
    portmidi, protobuf, qtbase, qtkeychain, qtsvg, rubberband,
    serd, sord, soundtouch, sratom, sqlite, taglib, upower, vamp-plugin-sdk, wavpack, gtest, gbenchmark, qtdeclarative, qt5compat, microsoft_gsl
}: stdenv.mkDerivation rec {
  name = "mixxx-${version}";
  # Reading the version from git output is very hard to do without wasting lots of diskspace and
  # runtime. Reading version file is easy.
  #version = lib.strings.removeSuffix "\"\n" (
  #            lib.strings.removePrefix "#define MIXXX_VERSION \"" (
  #              builtins.readFile ./src/_version.h ));

  # As of 2023-10-04, Mixxx gets its version number from CMake, which gets it from the latest Git tag.
  version = "2.4-beta-1172-gab8d11fc60-dirty";

  # SOURCE_DATE_EPOCH helps with python and pre-commit hook
  shellHook =  ''
    #export PYTHONPATH=venv/lib/python3.*/site-packages/:$PYTHONPATH
    #export SOURCE_DATE_EPOCH=315532800
    echo -e "Mixxx development shell. Available commands:\n"
    echo " configure - configures cmake (only has to run once)"
    echo " build - compiles Mixxx"
    echo " run - runs Mixxx with development settings"
    echo " debug - runs Mixxx inside gdb"
      '';

  src = if releaseMode then (nix-gitignore.gitignoreSource ''
    /cbuild
    /.envrc
    /result
    /shell.nix
    /venv
  '' ./.) else null;

  nativeBuildInputs = [
    cmake
  ] ++ (if !releaseMode then [
    ccache
    gdb
    git-clang-format
    clang-tools
    # for pre-commit installation since nixpkg.pre-commit may be to old
    #python3 python3.pkgs.virtualenv python3.pkgs.pip python3.pkgs.setuptools
    shell-configure shell-build shell-run shell-debug
  ] else []);

  buildInputs = [
    # buildInputs from <nixpkgs/pkgs/applications/audio/mixxx/default.nix>, Qt5->Qt6, removed qtscript and qtx11extras and added gtest, gbenchmark, qtdeclarative, qt5compat and microsoft_gsl for Qt6
    # as of https://github.com/NixOS/nixpkgs/commit/ab9cf80e428a8814cecb173efce840cc56b025eb
    chromaprint faad2 ffmpeg fftw flac glibcLocales hidapi lame libebur128
    libGLU libid3tag libkeyfinder libmad libmodplug libopus libsecret libshout
    libsndfile libusb1 libvorbis libxcb lilv lv2 mp4v2 opusfile pcre portaudio
    portmidi protobuf qtbase qtkeychain qtsvg rubberband
    serd sord soundtouch sratom sqlite taglib upower vamp-plugin-sdk wavpack gtest gbenchmark qtdeclarative qt5compat microsoft_gsl

    pkgs.microsoft_gsl
    pkgs.gbenchmark
  ] ++ allLv2Plugins;

  postInstall = (if releaseMode then ''
    wrapProgram $out/bin/mixxx --prefix LV2_PATH : ${lib.makeSearchPath "lib/lv2" allLv2Plugins}
  '' else "");

  meta = with lib; {
    homepage = https://mixxx.org;
    description = "Digital DJ mixing software";
    license = licenses.gpl2Plus;
    maintainers = pkgs.mixxx.meta.maintainers;
    platforms = platforms.linux;
  };
}) {}
