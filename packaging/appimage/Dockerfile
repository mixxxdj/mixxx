# Mixxx AppImage Builder
# Base image version is set via build arg (passed from build.sh).
# Debian 12 (bookworm) for 2.5.x, Debian 13 (trixie) for main/2.7.x.
ARG DEBIAN_VERSION=12
FROM debian:${DEBIAN_VERSION}-slim

# Install all build dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        # Build tools
        ccache \
        cmake \
        g++ \
        make \
        pkg-config \
        curl \
        ca-certificates \
        file \
        desktop-file-utils \
        # Audio libraries
        libavformat-dev \
        libebur128-dev \
        libfaad-dev \
        libfftw3-dev \
        libflac-dev \
        libmad0-dev \
        libmodplug-dev \
        libmp3lame-dev \
        libopus-dev \
        libopusfile-dev \
        libportmidi-dev \
        libsndfile1-dev \
        libsoundtouch-dev \
        libtag1-dev \
        libwavpack-dev \
        libchromaprint-dev \
        librubberband-dev \
        libshout-idjc-dev \
        # MIDI/HID/USB
        libhidapi-dev \
        libusb-1.0-0-dev \
        libudev-dev \
        # PipeWire / ALSA / JACK
        libpipewire-0.3-dev \
        libspa-0.2-dev \
        portaudio19-dev \
        # LV2
        liblilv-dev \
        lv2-dev \
        # Qt6 + QML
        qt6-base-dev \
        qt6-base-private-dev \
        qt6-declarative-dev \
        qt6-declarative-private-dev \
        qt6-multimedia-dev \
        qt6-shadertools-dev \
        qt6-5compat-dev \
        qt6-svg-dev \
        libqt6sql6-sqlite \
        qtkeychain-qt6-dev \
        qml6-module-qt5compat-graphicaleffects \
        qml6-module-qtqml-workerscript \
        qml6-module-qtquick-controls \
        qml6-module-qtquick-dialogs \
        qml6-module-qtquick-layouts \
        qml6-module-qtquick-shapes \
        qml6-module-qtquick-templates \
        qml6-module-qtquick-window \
        qml6-module-qt-labs-qmlmodels \
        qml6-module-qt-labs-folderlistmodel \
        qml6-module-qtmultimedia \
        # Other
        libid3tag0-dev \
        libmsgsl-dev \
        libssl-dev \
        libsqlite3-dev \
        libupower-glib-dev \
        protobuf-compiler \
        libprotobuf-dev \
        libgmock-dev \
        libgtest-dev \
        libbenchmark-dev \
        libgl1-mesa-dev \
        fonts-open-sans \
        libfuse2 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install libfuse (needed for appimagetool). Package name differs between
# Debian versions: libfuse2t64 on 13+, libfuse2 on 12.
RUN apt-get update && apt-get install -y --no-install-recommends libfuse2t64 2>/dev/null \
    || apt-get update && apt-get install -y --no-install-recommends libfuse2 2>/dev/null \
    || echo "Warning: libfuse not installed"

# Build Qt6 SVG image format plugin (not provided by Debian packages)
RUN \
    MULTIARCH=$(gcc -print-multiarch) && \
    mkdir -p /tmp/svgplugin && cd /tmp/svgplugin && \
    printf 'cmake_minimum_required(VERSION 3.16)\nproject(qsvg-plugin LANGUAGES CXX)\nset(CMAKE_AUTOMOC ON)\nfind_package(Qt6 REQUIRED COMPONENTS Svg Gui Core)\nadd_library(qsvg MODULE main.cpp)\ntarget_link_libraries(qsvg PRIVATE Qt6::Svg Qt6::Gui Qt6::Core)\nset_target_properties(qsvg PROPERTIES OUTPUT_NAME "qsvg")\ninstall(TARGETS qsvg DESTINATION lib/${MULTIARCH}/qt6/plugins/imageformats)\n' > CMakeLists.txt && \
    printf '#include <QImageIOPlugin>\n#include <QSvgRenderer>\n#include <QPainter>\n#include <QImage>\nclass QSvgPlugin : public QImageIOPlugin {\n    Q_OBJECT\n    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "qsvg.json")\npublic:\n    Capabilities capabilities(QIODevice *, const QByteArray &) const override;\n    QImageIOHandler *create(QIODevice *, const QByteArray & = QByteArray()) const override;\n};\nQImageIOPlugin::Capabilities QSvgPlugin::capabilities(QIODevice *device, const QByteArray &format) const {\n    if (format == "svg" || format == "svgz") return CanRead;\n    if (!format.isEmpty()) return {};\n    if (!device->isOpen()) return {};\n    QByteArray h = device->peek(4);\n    if (h.startsWith("<svg") || h.startsWith("<?xml")) return CanRead;\n    return {};\n}\nQImageIOHandler *QSvgPlugin::create(QIODevice *device, const QByteArray &) const {\n    QSvgRenderer r(device->readAll());\n    if (!r.isValid()) return nullptr;\n    QImage img(r.defaultSize(), QImage::Format_ARGB32);\n    img.fill(Qt::transparent);\n    QPainter p(&img); r.render(&p); p.end();\n    class H : public QImageIOHandler {\n    public:\n        explicit H(const QImage &i) : m_img(i) {}\n        bool canRead() const override { return true; }\n        bool read(QImage *i) override { *i = m_img; return true; }\n    private:\n        QImage m_img;\n    };\n    return new H(img);\n}\n#include "main.moc"\n' > main.cpp && \
    printf '{"Keys": ["svg", "svgz"]}\n' > qsvg.json && \
    mkdir -p build && cd build && \
    cmake /tmp/svgplugin -DCMAKE_PREFIX_PATH=/usr/lib/${MULTIARCH}/cmake -DCMAKE_INSTALL_PREFIX=/usr && \
    make -j$(nproc) && make install && \
    rm -rf /tmp/svgplugin && \
    echo "Qt6 SVG plugin built and installed"

# Download appimagetool into the image (avoids re-download per build)
# Uses ALL_PROXY build arg (passed from build.sh, only affects curl)
# Architecture is detected via uname -m and mapped to AppImage arch names
RUN ARCH=$(uname -m) && case "$ARCH" in \
        x86_64)  APPIMAGE_ARCH="x86_64"  ;; \
        aarch64) APPIMAGE_ARCH="aarch64" ;; \
        armv7l)  APPIMAGE_ARCH="armhf"   ;; \
        *) echo "Unsupported arch: $ARCH"; exit 1 ;; \
    esac && \
    mkdir -p /build/tools && \
    curl -fsSL --connect-timeout 15 --max-time 120 \
    -o "/build/tools/appimagetool-${APPIMAGE_ARCH}.AppImage" \
    "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${APPIMAGE_ARCH}.AppImage" && \
    chmod +x "/build/tools/appimagetool-${APPIMAGE_ARCH}.AppImage" && \
    echo "appimagetool downloaded for ${APPIMAGE_ARCH}"

# Create non-root builder user and ensure /build is writable
RUN useradd -m -u 1000 builder && \
    mkdir -p /build && \
    chown builder:builder /build

USER builder
WORKDIR /build

# Default environment
ENV CCACHE_DIR=/build/ccache \
    CCACHE_MAXSIZE=4G \
    LC_ALL=C.UTF-8 \
    LANG=C.UTF-8