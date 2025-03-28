{
  inputs = {
    utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:NixOS/nixpkgs/release-24.11";
  };
  outputs = { self, nixpkgs, utils }: utils.lib.eachDefaultSystem (system:
    let
      pkgs = nixpkgs.legacyPackages.${system};
    in
    {
      devShell = pkgs.mkShell {
        buildInputs = with pkgs; [
          # Building Mixxx
          qt6.full
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
        ];
        shellHook = ''
          pre-commit install
          pre-commit install -t pre-push
        '';
      };
    }
  );
}
