FROM ubuntu:24.04

RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get update && \
    apt-get install -y \
        build-essential \
        git \
        sudo \
        python3-pip \
        pulseaudio-utils \
	pipewire-alsa \
	pipewire \
	alsa-utils \
        clangd && \
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
    echo ". /etc/bash_completion\nexport PROMPT_COMMAND='history -a'\nexport HISTFILE='/home/developer/.devcontainer/.bash_history'" >> /home/developer/.bashrc
