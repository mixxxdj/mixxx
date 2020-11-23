{ nixroot ? (import <nixpkgs> { }), defaultLv2Plugins ? false, lv2Plugins ? [ ]
, releaseMode ? false, enableKeyfinder ? true, buildType ? "auto", cFlags ? [ ]
, useClang ? true }:
let
  inherit (nixroot)
    pkgs lib makeWrapper clang-tools cmake fetchurl fetchgit glibcLocales
    nix-gitignore python3 python37Packages;

  llvm_packages = pkgs.llvmPackages_10;
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

  cmakeBuildType = if buildType == "auto" then
    (if releaseMode then "RelWithDebInfo" else "Debug")
  else
    buildType;

  allCFlags = cFlags ++ [
    ("-DKEYFINDER=" + (if enableKeyfinder then "ON" else "OFF"))
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
    version = "1.7";
    name = "${shortname}-${version}";
    src = fetchurl {
      url = "https://github.com/KDE/clazy/archive/v${version}.tar.gz";
      sha256 =
        "01d595bc584e6b37be0a9b3db22a67aae6249dc5a05df10f50cb12ccbc3fc9a3";
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
    ((if useClang then ''
      if [ -e .github/workflows/clazy.yml ]; then
        echo "Using clazy"
        CXX=clazy
      fi
    '' else
      "") + ''
        mkdir -p cbuild
        cd cbuild
        cmake .. ${lib.escapeShellArgs allCFlags} "$@"
        cd ..
        V=$(pre-commit --version | sed s/"pre-commit /1.99.99\\n/" | sort -Vr | head -n1 | tr -d "\n")
        # install pre-commit from python for older systems
        if [ $V == "1.99.99" -a ! -e venv/bin/pre-commit ]; then
          virtualenv -p python3 venv
          ./venv/bin/pip install pre-commit
          ./venv/bin/pre-commit install
        else
          pre-commit install
        fi
        create-includes
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
    if [ ! -d "cbuild" ]; then
      >&2 echo "First you have to run configure."
      exit 1
    fi
    if [ -z "$CLAZY_CHECKS" -a -e .github/workflows/clazy.yml ]; then
      export CLAZY_CHECKS=$(grep CLAZY_CHECKS ./.github/workflows/clazy.yml | sed "s/\s*CLAZY_CHECKS:\s*//")
    fi
    cd cbuild
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
    if [ ! -f "cbuild/mixxx" ]; then
      >&2 echo "First you have to run build."
      exit 1
    fi
    cd cbuild
    exec ./mixxx --resourcePath res/ "$@"
  '';

  shell-debug = nixroot.writeShellScriptBin "debug" ''
    if [ ! -f "cbuild/mixxx" ]; then
      >&2 echo "First you have to run build."
      exit 1
    fi
    cd cbuild
    head -n1 mixxx | grep bash >/dev/null || (echo "mixxx is not wrapped" && exit 1)
    eval "$(head -n -1 mixxx)"
    exec gdb --args ./.mixxx-wrapped --resourcePath res/ "$@"
  '';

  shell-run-tests = nixroot.writeShellScriptBin "run-tests" ''
    if [ ! -f "cbuild/mixxx-test" ]; then
      >&2 echo "First you have to run build."
      exit 1
    fi
    cd cbuild
    # in case we run in a ci environment nix is < 20.09 QT_PLUGIN_PATH may not be set
    if [ -z "$QT_PLUGIN_PATH" ]; then
      export QT_PLUGIN_PATH=${pkgs.qt5.qtbase}/${pkgs.qt5.qtbase.qtPluginPrefix}
    fi
    ./mixxx-test "$@"
  '';

  # this creates a folder with links to the include folders of the configured
  # nix pkgs. This allows to IDE to look up the correct headers
  shell-create-includes = nixroot.writeShellScriptBin "create-includes" ''
    echo "Creating include cache"
    create_includes() {
      local IFS=:
      local includes
      set -f # Disable glob expansion
      includes=( $@ ) # Deliberately unquoted
      set +f
      mkdir -p cbuild/includes
      rm -f cbuild/includes/* >/dev/null
      cd cbuild/includes
      for i in "''${includes[@]}"
        do
            t=$(basename $(dirname $i))
            if [ ! -e $t ]; then
                ln -s -T $i ./$t
            fi
      done
    }
    create_includes $CMAKE_INCLUDE_PATH
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
    export PYTHONPATH=venv/lib/python3.7/site-packages/:$PYTHONPATH
    export SOURCE_DATE_EPOCH=315532800
    if [ -z $QT_MESSAGE_PATTERN ]; then
      QT_MESSAGE_PATTERN="`echo -e \"\033[32m%{time h:mm:ss.zzz}\033[0m \"`"
      QT_MESSAGE_PATTERN+="`echo -e \"%{if-category}\033[35m %{category}:\033[35m%{endif}\"`"
      QT_MESSAGE_PATTERN+="`echo -e \"[\033[97m{{threadname}}\033[0m] \"`"
      QT_MESSAGE_PATTERN+="`echo -e \"%{if-debug}\033[34m%{type} \033[36m%{function}%{endif}\"`"
      QT_MESSAGE_PATTERN+="`echo -e \"%{if-info}\033[32m%{type}%{endif}\"`"
      QT_MESSAGE_PATTERN+="`echo -e \"%{if-warning}\033[93m%{type}%{endif}\"`"
      QT_MESSAGE_PATTERN+="`echo -e \"%{if-critical}\033[91m%{type}%{endif}\"`"
      QT_MESSAGE_PATTERN+="`echo -e \"%{if-fatal}\033[97m\033[41m%{type} \033[30m%{file}:%{line}%{endif}\"`"
      QT_MESSAGE_PATTERN+="`echo -e \"\033[0m  %{message}\"`"
      export QT_MESSAGE_PATTERN
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
        # for pre-commit installation since nixpkg.pre-commit may be to old
        pre-commit
        python3
        python37Packages.pip
        python37Packages.setuptools
        python37Packages.virtualenv
        nodejs
        nixfmt
        shell-build
        shell-configure
        shell-debug
        shell-run
        shell-run-tests
        shell-create-includes
        nix
      ] ++ (if useClang then [ clazy ] else [ ]))
  else
    [ ]);

  cmakeFlags = allCFlags;

  buildInputs = (with pkgs; [
    chromaprint
    faad2
    ffmpeg
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
    qt5.qtbase
    qt5.qtdoc
    qt5.qtscript
    qt5.qtsvg
    qt5.qtx11extras
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
