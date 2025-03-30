FROM --platform=$BUILDPLATFORM ubuntu:24.04
ARG TARGETARCH

RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get update && \
    apt-get install -y \
    alsa-utils \
    # Includes basic building tools
    build-essential \
    ninja-build \
    # Used by most OpenSource LSPs extensions
    clangd \
    # Debugger
    gdb \
    # VCS
    git \
    # Audio integration
    pipewire \
    pipewire-alsa \
    pulseaudio-utils \
    # Required for pre-commit
    python3-pip \
    # Default Qt QPA
    qt6-wayland \
    # Allow privileges escalation in container (e.g to install custom deps)
    sudo \
    # Allow udev rules debug and test
    udev \
    # Convenience for git interactive commands
    vim && \
    pip3 install pre-commit --break-system-packages

COPY tools/debian_buildenv.sh /tmp/

RUN /tmp/debian_buildenv.sh setup

RUN userdel -f -r ubuntu && \
    mkdir /home/developer && echo "developer ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/developer && \
    mkdir -p \
    /home/developer/.cache/pre-commit \
    /home/developer/.devcontainer \
    /home/developer/.mixxx \
    /tmp/ccache && \
    touch /home/developer/.devcontainer/.bash_history && \
    chown -R 1000:1000 /home/developer/ /tmp/ccache && \
    echo ".cache\n.vscode\n.node_modules" >> /home/developer/.gitignore && \
    echo ". /etc/bash_completion\nexport PROMPT_COMMAND='history -a'\nexport HISTFILE='/home/developer/.devcontainer/.bash_history'" >> /home/developer/.bashrc
