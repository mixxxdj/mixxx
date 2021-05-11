{ nixroot ? (import <nixpkgs> { }), defaultLv2Plugins ? false, lv2Plugins ? [ ]
, releaseMode ? false, enableKeyfinder ? true, enableDjinterop ? false
, buildType ? "auto", cFlags ? [ ], useClang ? false, useClazy ? false
, buildFolder ? "build", qtDebug ? false }:
let
  inherit (nixroot)
    pkgs lib makeWrapper clang-tools cmake fetchurl fetchgit glibcLocales
    nix-gitignore python3 python37Packages;

  llvm_packages = if builtins.hasAttr "llvmPackages_10" pkgs then
    pkgs.llvmPackages_10
  else
    pkgs.llvmPackages;

  clangVersion = builtins.map builtins.fromJSON
    (lib.versions.splitVersion llvm_packages.clang.version);
  glibcVersion = builtins.map builtins.fromJSON
    (lib.versions.splitVersion pkgs.glibc.version);

  # llvm < 10.0.2 and glibc >= 2.31 do not work with -fast-math
  fastMathBug = (builtins.elemAt clangVersion 0) <= 10
    && (builtins.elemAt clangVersion 1) <= 0 && (builtins.elemAt clangVersion 2)
    < 2 && (builtins.elemAt glibcVersion 0) <= 2
    && (builtins.elemAt glibcVersion 1) >= 31;

  stdenv = if useClang then llvm_packages.stdenv else pkgs.stdenv;

  qtDeps = if qtDebug then [
    (pkgs.enableDebugging ((pkgs.qt5.override {
      debug = true;
      developerBuild = true;
    }).qtbase))
    pkgs.qt5.qtscript
    pkgs.qt5.qtsvg
    pkgs.qt5.qtx11extras
    pkgs.qt5.qtdoc
  ] else [
    pkgs.qt5.qtbase
    pkgs.qt5.qtdoc
    pkgs.qt5.qtscript
    pkgs.qt5.qtsvg
    pkgs.qt5.qtx11extras
  ];

  cmakeBuildType = if buildType == "auto" then
    (if releaseMode then "RelWithDebInfo" else "Debug")
  else
    buildType;

  allCFlags = cFlags ++ [
    ("-DKEYFINDER=" + (if enableKeyfinder then "ON" else "OFF"))
    ("-DENGINEPRIME=" + (if enableDjinterop then "ON" else "OFF"))
    ("-DCMAKE_BUILD_TYPE=" + cmakeBuildType)
  ] ++ (if useClang && fastMathBug then [ "-DFAST_MATH=off" ] else [ ]);

  git-clang-format = pkgs.stdenv.mkDerivation {
    name = "git-clang-format";
    version = "2019-06-21";
    src = fetchurl {
      url =
        "https://raw.githubusercontent.com/llvm-mirror/clang/2bb8e0fe002e8ffaa9ce5fa58034453c94c7e208/tools/clang-format/git-clang-format";
      sha256 = "1kby36i80js6rwi11v3ny4bqsi6i44b9yzs23pdcn9wswffx1nlf";
      executable = true;
    };
    nativeBuildInputs = [ makeWrapper ];
    buildInputs = [ clang-tools python3 ];
    unpackPhase = ":";
    installPhase = ''
      mkdir -p $out/opt $out/bin
      cp $src $out/opt/git-clang-format
      makeWrapper $out/opt/git-clang-format $out/bin/git-clang-format \
        --add-flags --binary \
        --add-flags ${clang-tools}/bin/clang-format
    '';
  };

  clazy = stdenv.mkDerivation rec {
    shortname = "clazy";
    version = "1.9";
    name = "${shortname}-${version}";
    src = fetchurl {
      url = "https://github.com/KDE/clazy/archive/v${version}.tar.gz";
      sha256 = "155pyx8mcijchmchiny1lnilb3b0nhbrxcnxksvhcyy1vjnpghsn";
    };
    cmakeFlags = [ "-DCMAKE_MODULE_PATH=${llvm_packages.clang}" ];
    nativeBuildInputs = [
      cmake
      makeWrapper
      llvm_packages.llvm
      llvm_packages.clang
      llvm_packages.clang-unwrapped
    ];
    buildInputs = [ clang-tools cmake ];
  };

  libdjinterop = stdenv.mkDerivation rec {
    shortname = "libdjinterop";
    version = "0.15.1";
    name = "${shortname}-${version}";
    src = fetchurl {
      url = "https://github.com/xsco/libdjinterop/archive/${version}.tar.gz";
      sha256 = "02607xxy3zzyh75a0g6vksnwjc3j1apy7rxpalyk62624v3ydcw7";
    };
    nativeBuildInputs = [ cmake makeWrapper pkgs.zlib pkgs.sqlite ];
    # buildInputs = [ clang-tools cmake ];
  };

  libkeyfinder = if builtins.hasAttr "libkeyfinder" pkgs then
    pkgs.libkeyfinder
  else
    pkgs.stdenv.mkDerivation {
      name = "libkeyfinder";
      version = "2.2.1-dev";

      src = fetchgit {
        url = "https://github.com/ibsh/libKeyFinder.git";
        rev = "9b4440d66789b06483fe5273e06368a381d22707";
        sha256 = "008xg6v4mpr8hqqkn8r5y5vnigggnqjjcrhv5r6q41xg6cfz0k72";
      };

      nativeBuildInputs = [ cmake ];
      buildInputs = with pkgs; [ fftw pkgconfig ];

      buildPhase = ''
        make VERBOSE=1 keyfinder
      '';
      installPhase = ''
        make install/fast
      '';
    };

  shell-configure = nixroot.writeShellScriptBin "configure"
    ((if useClazy then ''
      if [ -e .github/workflows/clazy.yml ]; then
        echo "Using clazy"
        CXX=clazy
      fi
    '' else
      "") + ''
        MIXXX_SRC=$(pwd)
        mkdir -p ${buildFolder}
        cd ${buildFolder}
        cmake $MIXXX_SRC ${
          lib.escapeShellArgs allCFlags
        } -DCMAKE_EXPORT_COMPILE_COMMANDS=ON "$@"
        cd -
        link-environment
      '');

  wrapper = (if builtins.hasAttr "wrapQtAppsHook" pkgs.qt5 then
    "qt5.wrapQtAppsHook"
  else
    "pkgs.makeWrapper");

  wrapperCmd = (if builtins.hasAttr "wrapQtAppsHook" pkgs.qt5 then
    "wrapQtApp"
  else
    "wrapProgram");

  # sourceing stdenv/setup and qt5.wrapQtAppsHook does not work
  # we therefore need to call the wrapper script through a minimal nix-shell
  shell-build = nixroot.writeShellScriptBin "build" ''
    set -e
    if [ ! -d "${buildFolder}" ]; then
      >&2 echo "First you have to run configure."
      exit 1
    fi
    if [ -z "$CLAZY_CHECKS" -a -e .github/workflows/build-checks.yml ]; then
      export CLAZY_CHECKS=$(grep CLAZY_CHECKS ./.github/workflows/build-checks.yml  | sed "s/\s*CLAZY_CHECKS:\s*//")
      echo "Using clazy checks: $CLAZY_CHECKS"
    fi
    cd ${buildFolder}
    rm -f .mixxx-wrapped mixxx mixxx-test .mixxx-test-wrapped
    cmake --build . --parallel $NIX_BUILD_CORES "$@"
    if [ -f ./mixxx ]; then
      echo "run ${wrapperCmd} mixxx"
      env shellHook="" nix-shell -p ${wrapper} --command "${wrapperCmd} mixxx --prefix LV2_PATH : ${
        lib.makeSearchPath "lib/lv2" allLv2Plugins
      } --prefix LD_LIBRARY_PATH ${pkgs.faad2}/lib"
    fi
    if [ -f ./mixxx-test ]; then
      echo "run ${wrapperCmd} mixxx-test"
      env shellHook="" nix-shell -p ${wrapper} --command "${wrapperCmd} mixxx-test --prefix LD_LIBRARY_PATH : ${pkgs.faad2}/lib"
    fi
  '';

  shell-run = nixroot.writeShellScriptBin "run" ''
    if [ ! -f "${buildFolder}/mixxx" ]; then
      >&2 echo "First you have to run build."
      exit 1
    fi
    cd ${buildFolder}
    exec ./mixxx --resourcePath ../res/ "$@"
  '';

  shell-debug = nixroot.writeShellScriptBin "debug" ''
    if [ ! -f "${buildFolder}/mixxx" ]; then
      >&2 echo "First you have to run build."
      exit 1
    fi
    cd ${buildFolder}
    head -n1 mixxx | grep bash >/dev/null || (echo "mixxx is not wrapped" && exit 1)
    eval "$(head -n -1 mixxx)"
    exec gdb --args ./.mixxx-wrapped --resourcePath ../res/ "$@"
  '';

  shell-run-tests = nixroot.writeShellScriptBin "run-tests" ''
    if [ ! -f "${buildFolder}/mixxx-test" ]; then
      >&2 echo "First you have to run build."
      exit 1
    fi
    cd ${buildFolder}
    # in case we run in a ci environment nix is < 20.09 QT_PLUGIN_PATH may not be set
    if [ -z "$QT_PLUGIN_PATH" ]; then
      export QT_PLUGIN_PATH=${pkgs.qt5.qtbase}/${pkgs.qt5.qtbase.qtPluginPrefix}
    fi
    if [ -z "$QT_QPA_PLATFORM" ]; then
      export QT_QPA_PLATFORM=offscreen
    fi
    ./mixxx-test "$@"
  '';

  # this creates a folder with links to the include folders of the configured
  # nix pkgs. This allows to IDE to look up the correct headers
  shell-link-environment = nixroot.writeShellScriptBin "link-environment" ''
    echo "Link environment"
    create_includes() {
      local IFS=:
      local includes
      set -f # Disable glob expansion
      includes=( $@ ) # Deliberately unquoted
      set +f
      mkdir -p ${buildFolder}/includes
      rm -f ${buildFolder}/includes/* >/dev/null
      cd ${buildFolder}/includes
      for i in "''${includes[@]}"
        do
            t=$(basename $(dirname $i))
            if [ ! -e $t ]; then
                ln -s -T $i ./$t
            fi
      done
      cd ../..
    }
    create_includes $CMAKE_INCLUDE_PATH
    rm -f ${buildFolder}/compiler 2>/dev/null
    ln -s ${stdenv.cc} ${buildFolder}/compiler
  '';

  allLv2Plugins = lv2Plugins ++ (if defaultLv2Plugins then
    (with pkgs; [
      artyFX
      infamousPlugins
      libsepol
      mod-distortion
      mount
      pcre
      rkrlv2
      utillinux
      x42-plugins
      zam-plugins
      sratom
    ])
  else
    [ ]);

in stdenv.mkDerivation rec {
  name = "mixxx-${version}";
  # Reading the version from git output is very hard to do without wasting lots of diskspace and
  # runtime. Reading version file is easy.
  version = lib.strings.removeSuffix ''
    "
  '' (lib.strings.removePrefix ''#define MIXXX_VERSION "''
    (builtins.readFile ./src/_version.h));

  # SOURCE_DATE_EPOCH helps with python and pre-commit hook
  shellHook = ''
    if [ -z "$LOCALE_ARCHIVE" ]; then
      export LOCALE_ARCHIVE="${glibcLocales}/lib/locale/locale-archive";
    fi
    echo -e "Mixxx development shell. Available commands:\n"
    echo " configure - configures cmake (only has to run once)"
    echo " build - compiles Mixxx"
    echo " run - runs Mixxx with development settings"
    echo " debug - runs Mixxx inside gdb"
    echo " run-tests - runs Mixxx tests"
      '';

  src = if releaseMode then
    (nix-gitignore.gitignoreSource ''
      /cbuild
      /${buildFolder}
      /.envrc
      /result*
      /shell.nix
      /venv
    '' ./.)
  else
    null;

  nativeBuildInputs = [ pkgs.cmake ] ++ (if !releaseMode then
    (with pkgs;
      [
        ccache
        clang-tools
        gdb
        git-clang-format
        glibcLocales
        include-what-you-use
        python3
        nodejs
        shell-build
        shell-configure
        shell-debug
        shell-run
        shell-run-tests
        shell-link-environment
        nix
        setupDebugInfoDirs
      ] ++ lib.optional useClang [ clazy ]
      ++ lib.optional (builtins.hasAttr "pre-commit" pkgs) [ pkgs.pre-commit ]
      ++ lib.optional (builtins.hasAttr "nixfmt" pkgs) [ pkgs.nixfmt ])
  else
    [ ]);

  cmakeFlags = allCFlags;

  buildInputs = qtDeps ++ (with pkgs; [
    chromaprint
    faad2
    ffmpeg_4
    fftw
    flac
    git
    glib
    hidapi
    lame
    libebur128
    libGLU
    libid3tag
    libmad
    libmodplug
    libopus
    libshout
    libsecret
    libselinux
    libsepol
    libsForQt5.qtkeychain
    libsndfile
    libusb1
    libvorbis
    lilv
    lv2
    makeWrapper
    mp4v2
    opusfile
    pcre
    pkgconfig
    portaudio
    portmidi
    protobuf
    rubberband
    serd
    sord
    soundtouch
    sqlite
    taglib
    upower
    utillinux
    vamp.vampSDK
    wavpack
    x11
  ]) ++ allLv2Plugins ++ (if enableKeyfinder then [ libkeyfinder ] else [ ])
    ++ lib.optionals enableDjinterop [ libdjinterop ]
    ++ (if builtins.hasAttr "wrapQtAppsHook" pkgs.qt5 then
      [ pkgs.qt5.wrapQtAppsHook ]
    else
      [ ]);

  postInstall = (if releaseMode then ''
    ${wrapperCmd} $out/bin/mixxx --prefix LV2_PATH : ${
      lib.makeSearchPath "lib/lv2" allLv2Plugins
    } --prefix LD_LIBRARY_PATH ${pkgs.faad2}/lib
  '' else
    "");

  meta = with nixroot.stdenv.lib; {
    homepage = "https://mixxx.org";
    description = "Digital DJ mixing software";
    license = licenses.gpl2Plus;
    maintainers = nixroot.pkgs.mixxx.meta.maintainers;
    platforms = platforms.linux;
  };
}
