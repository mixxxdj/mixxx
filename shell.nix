{ nixroot  ? (import <nixpkgs> {})
, defaultLv2Plugins ? false
, lv2Plugins ? []
}:
let inherit (nixroot) stdenv pkgs lib
    chromaprint fftw flac libid3tag libmad libopus libshout libsndfile lilv
    libusb1 libvorbis libebur128 pkgconfig portaudio portmidi protobuf qt5 glib
    rubberband sqlite taglib soundtouch vamp opusfile hidapi upower ccache git
    libGLU x11 lame lv2 makeWrapper
    clang-tools
    cmake
    fetchurl
    ffmpeg
    gdb
    libmodplug
    mp4v2
    nix-gitignore
    python3
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
  '';

  shell-build = nixroot.writeShellScriptBin "build" ''
    if [ ! -d "cbuild" ]; then
      >&2 echo "First you have to run configure."
      exit 1
    fi
    cd cbuild
    cmake --build . --parallel $NIX_BUILD_CORES "$@"
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
    gdb --args ./mixxx --resourcePath res/ "$@"
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

  shellHook =  ''
    echo -e "Mixxx development shell. Available commands:\n"
    echo " configure - configures cmake (only has to run once)"
    echo " build - compiles Mixxx"
    echo " run - runs Mixxx with development settings"
    echo " debug - runs Mixxx inside gdb"
      '';

  src = nix-gitignore.gitignoreSource ''
    /cbuild
  '' ./.;

  nativeBuildInputs = [
    ccache # If you want to build Mixxx as a derivation, then you have to remove ccache here.
    cmake
    gdb
    git-clang-format
    shell-configure shell-build shell-run shell-debug
  ];

  buildInputs = [
    chromaprint fftw flac libid3tag libmad libopus libshout libsndfile
    libusb1 libvorbis libebur128 pkgconfig portaudio portmidi protobuf qt5.full
    rubberband sqlite taglib soundtouch vamp.vampSDK opusfile upower hidapi
    git glib x11 libGLU lilv lame lv2 makeWrapper qt5.qtbase
    ffmpeg
    libmodplug
    mp4v2
    wavpack
  ] ++ allLv2Plugins;

  meta = with nixroot.stdenv.lib; {
    homepage = https://mixxx.org;
    description = "Digital DJ mixing software";
    license = licenses.gpl2Plus;
    maintainers = [ maintainers.aszlig maintainers.goibhniu ];
    platforms = platforms.linux;
  };
}
