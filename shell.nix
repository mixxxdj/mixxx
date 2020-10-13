{ nixroot  ? (import <nixpkgs> {})
, defaultLv2Plugins ? false
, lv2Plugins ? []
, releaseMode ? false
}:
let inherit (nixroot) stdenv pkgs lib
    chromaprint fftw flac libid3tag libmad libopus libshout libsndfile lilv
    libusb1 libvorbis libebur128 pkgconfig portaudio portmidi protobuf qt5 glib
    rubberband sqlite taglib soundtouch vamp opusfile hidapi upower ccache git
    libGLU x11 lame lv2 makeWrapper pcre utillinux libselinux libsepol
    libsForQt5
    clang-tools
    cmake
    fetchurl
    ffmpeg
    gdb
    libmodplug
    mp4v2
    nix-gitignore
    python3 python37Packages
    wavpack;

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

  shell-configure = nixroot.writeShellScriptBin "configure" ''
    mkdir -p cbuild
    cd cbuild
    cmake .. "$@"
    cd ..
    if [ ! -e venv/bin/pre-commit ]; then
      virtualenv -p python3 venv
      ./venv/bin/pip install pre-commit
      ./venv/bin/pre-commit install
    fi
  '';

  shell-build = nixroot.writeShellScriptBin "build" ''
    if [ ! -d "cbuild" ]; then
      >&2 echo "First you have to run configure."
      exit 1
    fi
    cd cbuild
    cmake --build . --parallel $NIX_BUILD_CORES "$@"
    source ${pkgs.makeWrapper}/nix-support/setup-hook
    wrapProgram mixxx --prefix LV2_PATH : ${lib.makeSearchPath "lib/lv2" allLv2Plugins}
  '';

  shell-run = nixroot.writeShellScriptBin "run" ''
    if [ ! -f "cbuild/mixxx" ]; then
      >&2 echo "First you have to run build."
      exit 1
    fi
    cd cbuild
    ./mixxx --resourcePath res/ "$@"
  '';

  shell-debug = nixroot.writeShellScriptBin "debug" ''
    if [ ! -f "cbuild/mixxx" ]; then
      >&2 echo "First you have to run build."
      exit 1
    fi
    cd cbuild
    LV2_PATH=${lib.makeSearchPath "lib/lv2" allLv2Plugins} gdb --args ./.mixxx-wrapped --resourcePath res/ "$@"
  '';

  allLv2Plugins = lv2Plugins ++ (if defaultLv2Plugins then [
    nixroot.x42-plugins nixroot.zam-plugins nixroot.rkrlv2 nixroot.mod-distortion
    nixroot.infamousPlugins nixroot.artyFX
  ] else []);

in stdenv.mkDerivation rec {
  name = "mixxx-${version}";
  # Reading the version from git output is very hard to do without wasting lots of diskspace and
  # runtime. Reading version file is easy.
  version = lib.strings.removeSuffix "\"\n" (
              lib.strings.removePrefix "#define MIXXX_VERSION \"" (
                builtins.readFile ./src/_version.h ));

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
    python3 python37Packages.virtualenv python37Packages.pip python37Packages.setuptools
    shell-configure shell-build shell-run shell-debug
  ] else []);

  buildInputs = [
    chromaprint fftw flac libid3tag libmad libopus libshout libsndfile
    libusb1 libvorbis libebur128 pkgconfig portaudio portmidi protobuf qt5.full
    rubberband sqlite taglib soundtouch vamp.vampSDK opusfile upower hidapi
    git glib x11 libGLU lilv lame lv2 makeWrapper qt5.qtbase pcre utillinux libselinux
    libsepol libsForQt5.qtkeychain
    ffmpeg
    libmodplug
    mp4v2
    wavpack
  ] ++ allLv2Plugins;

  postInstall = (if releaseMode then ''
    wrapProgram $out/bin/mixxx --prefix LV2_PATH : ${lib.makeSearchPath "lib/lv2" allLv2Plugins}
  '' else "");

  meta = with nixroot.stdenv.lib; {
    homepage = https://mixxx.org;
    description = "Digital DJ mixing software";
    license = licenses.gpl2Plus;
    maintainers = nixroot.pkgs.mixxx.meta.maintainers;
    platforms = platforms.linux;
  };
}
