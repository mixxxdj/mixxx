{
  inputs = {
    utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:NixOS/nixpkgs/release-24.11";
  };
  outputs = { self, nixpkgs, utils }: utils.lib.eachDefaultSystem (system:
    let
      pkgs = nixpkgs.legacyPackages.${system};
      qt6Env = with pkgs.qt6; env "qt-custom-${qtbase.version}"
        [
          qt5compat
          qtshadertools
          qtsvg
          qtdeclarative
        ];
    in
    {
      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          # Building Mixxx
          qt6Env
          cmake
          chromaprint
          glib
          libebur128
          fftw
          flac
          lame
          libogg
          libvorbis
          portaudio
          portmidi
          protobuf
          rubberband
          libsndfile
          soundtouch
          taglib
          upower
          openssl
          microsoft-gsl
          kdePackages.qtkeychain
          hidapi
          wavpack
          libid3tag
          libusb1
          libmad
          libopus
          opusfile
          libshout
          lilv
          libxkbcommon
          sqlite
          gtest
          clang-tools
          mp4v2
          vulkan-loader
          xorg.libX11
          ffmpeg
          libmodplug
          vamp-plugin-sdk
          ccache
          libGLU
          pcre
          libselinux
          utillinux
          libdjinterop
          libkeyfinder
          cups
          lv2

          # Git pre-commits
          pre-commit
          nodejs
          rustup
          stdenv.cc.cc
        ];
        shellHook = ''
          pre-commit install
          pre-commit install -t pre-push
          # Needed for clang-format pre-commit because it downloads and executes its own clang-format elf-binary
          export LD_LIBRARY_PATH="${pkgs.stdenv.cc.cc.lib}/lib/:$LD_LIBRARY_PATH"
        '';
      };
    }
  );
}
