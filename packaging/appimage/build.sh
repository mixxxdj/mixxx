#!/bin/bash
# ============================================================
# Mixxx AppImage Build Script
# ============================================================
# Builds a Mixxx AppImage entirely inside Docker.
#
# Usage:
#   bash packaging/appimage/build.sh                    # Build AppImage
#   bash packaging/appimage/build.sh clean              # Remove build artifacts
#   bash packaging/appimage/build.sh distclean          # Remove artifacts + Docker image
#   HOST_PROJECT_DIR=/path bash packaging/appimage/build.sh  # From inside Docker
#
# Environment variables:
#   HOST_PROJECT_DIR   - Host path to project source (required when inside Docker)
#   SOCKS5_PROXY       - SOCKS5 proxy URL (e.g. socks5://localhost:1080 or socks5://host.docker.internal:1080)
#   BUILD_TYPE         - CMake build type (default: RelWithDebInfo)
#   OUTPUT_DIR         - Host path for AppImage output (default: build/appimage)
#   BUILD_JOBS         - Parallel build jobs (default: nproc)
# ============================================================
set -euo pipefail

# ============================================================
# CONFIGURATION — edit these values as needed
# ============================================================

# Debian version for the build container.
# Debian 12 (bookworm) for 2.5.x, Debian 13 (trixie) for main/2.7.x.
DEBIAN_VERSION="13"

# Docker image name and tag
IMAGE_NAME="mixxx-appimage-builder:debian${DEBIAN_VERSION}"

# SOCKS5 proxy: uncomment to enable (needed for GitHub access in some networks).
# Leave commented to disable proxy (direct connection).
# SOCKS5_PROXY="socks5://localhost:1080"

# Host path to project source (required when running build script inside Docker)
# HOST_PROJECT_DIR="/home/user/mixxx"

# Build type (RelWithDebInfo, Release, Debug)
BUILD_TYPE="${BUILD_TYPE:-RelWithDebInfo}"

# Parallel build jobs (default: number of CPU cores)
BUILD_JOBS="${BUILD_JOBS:-$(nproc)}"

# ============================================================
# END CONFIGURATION
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
OUTPUT_DIR="${OUTPUT_DIR:-${PROJECT_DIR}/build/appimage}"

# ---- Subcommand dispatch ----
case "${1:-}" in
    clean)
        echo "Cleaning AppImage build artifacts..."
        rm -rf "${OUTPUT_DIR}"
        echo "Done"
        exit 0
        ;;
    distclean)
        echo "Cleaning all AppImage build artifacts..."
        rm -rf "${OUTPUT_DIR}"
        docker rmi "${IMAGE_NAME}" 2>/dev/null || true
        echo "Done"
        exit 0
        ;;
    "")
        # Proceed with build
        ;;
    *)
        echo "ERROR: Unknown subcommand: $1"
        echo "Usage: $0 [clean|distclean]"
        exit 1
        ;;
esac

# ---- Architecture detection ----
ARCH=$(uname -m)
case "$ARCH" in
    x86_64)  APPIMAGE_ARCH="x86_64"  ;;
    aarch64) APPIMAGE_ARCH="aarch64" ;;
    armv7l)  APPIMAGE_ARCH="armhf"   ;;
    *)
        echo "ERROR: Unsupported architecture: $ARCH"
        exit 1
        ;;
esac
echo "Detected architecture: $ARCH (AppImage arch: $APPIMAGE_ARCH)"

# ---- Configuration (overridable via env vars) ----
SOCKS5_PROXY="${SOCKS5_PROXY:-}"
BUILD_TYPE="${BUILD_TYPE:-RelWithDebInfo}"
OUTPUT_DIR="${OUTPUT_DIR:-${PROJECT_DIR}/build/appimage}"
BUILD_JOBS="${BUILD_JOBS:-$(nproc)}"

# ============================================================
# INNER: Build inside Docker container
# ============================================================
if [ "${INSIDE_MIXXX_BUILDER:-}" = "1" ]; then
    echo ""
    echo "=== [INNER] Starting AppImage build ==="
    echo "Architecture: ${ARCH} (${APPIMAGE_ARCH})"
    echo "Build type: ${BUILD_TYPE}"
    echo "Build jobs: ${BUILD_JOBS}"
    echo "Source: /src"
    echo "Output: /build/output"
    echo ""

    # Ensure required directories
    mkdir -p /build/{build,AppDir,output,tools}

    # ---- Step 1: Verify appimagetool ----
    APPIMAGETOOL="/build/tools/appimagetool-${APPIMAGE_ARCH}.AppImage"
    if [ ! -f "$APPIMAGETOOL" ]; then
        echo "ERROR: appimagetool not found at ${APPIMAGETOOL}"
        echo "  It should have been downloaded during Docker image build."
        exit 1
    fi

    # ---- Step 2: Configure CMake ----
    echo ""
    echo "=== Configuring CMake ==="
    cmake -S /src -B /build/build \
        -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DQT6=ON \
        -DBUILD_TESTING=OFF \
        -DBUILD_BENCH=OFF \
        -DINSTALL_USER_UDEV_RULES=OFF \
        -DWARNINGS_FATAL=OFF \
        -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
        -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O1 -g -DNDEBUG"

    # ---- Step 3: Build ----
    echo ""
    echo "=== Building Mixxx ==="
    cmake --build /build/build --parallel "${BUILD_JOBS}"

    # ---- Step 4: Install to AppDir ----
    echo ""
    echo "=== Installing to AppDir ==="
    DESTDIR=/build/AppDir cmake --install /build/build

    # ---- Step 5: Strip debug symbols ----
    echo ""
    echo "=== Stripping debug symbols ==="
    set +e
    strip --strip-unneeded "/build/AppDir/usr/bin/mixxx" 2>/dev/null
    find /build/AppDir/usr/lib -name "*.so*" -type f -exec strip --strip-unneeded {} \; 2>/dev/null
    set -e
    echo "  Stripped binary and libraries"

    # ---- Step 6: Bundle dependencies ----
    echo ""
    echo "=== Bundling dependencies ==="
    bash /src/packaging/appimage/bundle.sh /build/AppDir

    # ---- Step 7: Create AppRun entry point ----
    echo ""
    echo "=== Creating AppRun ==="
    cp /src/packaging/appimage/AppRun.sh /build/AppDir/AppRun
    chmod +x /build/AppDir/AppRun

    # ---- Step 8: Prepare desktop file ----
    echo "=== Preparing desktop file ==="
    # Desktop file is pre-processed (Exec=mixxx) in packaging/appimage/
    cp /src/packaging/appimage/org.mixxx.Mixxx.desktop /build/AppDir/org.mixxx.Mixxx.desktop
    chmod 644 /build/AppDir/org.mixxx.Mixxx.desktop

    # ---- Step 9: Prepare icon ----
    echo "=== Preparing icon ==="
    cp /src/res/images/icons/256x256/apps/mixxx.png /build/AppDir/mixxx.png
    ln -sf mixxx.png /build/AppDir/.DirIcon

    # ---- Step 10: Create AppImage ----
    echo ""
    echo "=== Creating AppImage ==="
    cd /build

    # Extract version from CMakeLists.txt
    MIXXX_VERSION=$(grep -oP 'project\(mixxx VERSION \K[^ )]+' /src/CMakeLists.txt)
    PRERELEASE=$(grep -oP 'set\(MIXXX_VERSION_PRERELEASE "\K[^"]+' /src/CMakeLists.txt || true)
    if [ -n "$PRERELEASE" ]; then
        VERSION="${MIXXX_VERSION}-${PRERELEASE}"
    else
        VERSION="${MIXXX_VERSION}"
    fi
    export VERSION

    OUTPUT_APPIMAGE="Mixxx-${VERSION}-${APPIMAGE_ARCH}.AppImage"

    # Run appimagetool (with APPIMAGE_EXTRACT_AND_RUN=1 to work without FUSE in Docker)
    APPIMAGE_EXTRACT_AND_RUN=1 \
    "$APPIMAGETOOL" \
        /build/AppDir \
        "/build/output/${OUTPUT_APPIMAGE}"

    # ---- Step 11: Verify ----
    echo ""
    echo "=== AppImage build complete ==="
    if [ -f "/build/output/${OUTPUT_APPIMAGE}" ]; then
        ls -lh "/build/output/${OUTPUT_APPIMAGE}"
        file "/build/output/${OUTPUT_APPIMAGE}"
    else
        echo "ERROR: AppImage was not created!"
        exit 1
    fi

    exit 0
fi

# ============================================================
# OUTER: Host-side orchestration
# ============================================================

echo ""
echo "=== Mixxx AppImage Builder ==="
echo "Host architecture: ${ARCH} (${APPIMAGE_ARCH})"
echo ""

# Check Docker availability
if ! command -v docker &>/dev/null; then
    echo "ERROR: Docker is not installed. Please install Docker first."
    echo "  See: https://docs.docker.com/engine/install/"
    exit 1
fi

# Check Docker daemon
if ! docker info &>/dev/null; then
    echo "ERROR: Docker daemon is not accessible."
    echo "  Ensure the Docker socket is available (Docker must be running)."
    echo "  You may need to be in the docker group."
    exit 1
fi

# ---- Detect Docker-in-Docker and resolve host paths ----
INSIDE_DOCKER=false
if [ -f /.dockerenv ] || [ -f /run/.containerenv ]; then
    INSIDE_DOCKER=true
fi
if ! $INSIDE_DOCKER && grep -qE 'docker|containerd|kubepods' /proc/1/cgroup 2>/dev/null; then
    INSIDE_DOCKER=true
fi

# Determine the host path for the project source
if [ -n "${HOST_PROJECT_DIR:-}" ]; then
    HOST_SRC_DIR="$HOST_PROJECT_DIR"
    echo "Using HOST_PROJECT_DIR: ${HOST_SRC_DIR}"
elif $INSIDE_DOCKER; then
    echo ""
    echo "ERROR: Detected running inside a Docker container."
    echo ""
    echo "  The current working directory (/work) is inside the container."
    echo "  To mount the source into the build container, we need the"
    echo "  corresponding path on the HOST filesystem."
    echo ""
    echo "  Please set HOST_PROJECT_DIR to the host path of the project:"
    echo ""
    echo "    HOST_PROJECT_DIR=/path/to/mixxx bash packaging/appimage/build.sh"
    echo ""
    echo "  For example:"
    echo "    HOST_PROJECT_DIR=/home/user/mixxx bash packaging/appimage/build.sh"
    echo ""
    exit 1
else
    HOST_SRC_DIR="$PROJECT_DIR"
    echo "Running on host. Source directory: ${HOST_SRC_DIR}"
fi

if [ -n "${SOCKS5_PROXY:-}" ]; then
    echo "Using SOCKS5_PROXY: ${SOCKS5_PROXY}"
else
    echo "No SOCKS5 proxy configured. GitHub downloads will go direct."
fi

# ---- Build Docker image ----
echo ""
echo "=== Building Docker image: ${IMAGE_NAME} ==="
DOCKER_BUILD_ARGS=(
    --build-arg "DEBIAN_VERSION=${DEBIAN_VERSION}"
)
if [ -n "${SOCKS5_PROXY:-}" ]; then
    DOCKER_BUILD_ARGS+=(--network host)
    DOCKER_BUILD_ARGS+=(--build-arg "ALL_PROXY=${SOCKS5_PROXY}")
fi
docker build "${DOCKER_BUILD_ARGS[@]}" \
    -t "${IMAGE_NAME}" \
    "${SCRIPT_DIR}"

# ---- Prepare directories ----
mkdir -p "${OUTPUT_DIR}"

# Determine the host-side path for the output mount.
# On the host: OUTPUT_DIR is already a valid host path.
# In Docker-in-Docker: use HOST_PROJECT_DIR to construct the host path.
HOST_OUTPUT_DIR="${OUTPUT_DIR}"
if $INSIDE_DOCKER && [ -n "${HOST_PROJECT_DIR:-}" ]; then
    HOST_OUTPUT_DIR="${HOST_PROJECT_DIR}/build/appimage"
fi
# Docker daemon will create the mount directory if it doesn't exist

# ---- Prepare Docker run arguments ----
DOCKER_RUN_ARGS=(
    --rm
    -t
    -v "${HOST_SRC_DIR}:/src:ro"
    -v "${HOST_OUTPUT_DIR}:/build/output"
    -e "INSIDE_MIXXX_BUILDER=1"
    -e "BUILD_TYPE=${BUILD_TYPE}"
    -e "BUILD_JOBS=${BUILD_JOBS}"
)

# --network host so localhost:1080 proxy works (needed for CMake downloads)
if [ -n "${SOCKS5_PROXY:-}" ]; then
    DOCKER_RUN_ARGS+=(--network host)
    for var in ALL_PROXY all_proxy HTTP_PROXY HTTPS_PROXY; do
        DOCKER_RUN_ARGS+=(-e "${var}=${SOCKS5_PROXY}")
    done
fi

# ---- Run build ----
echo ""
echo "=== Starting AppImage build in Docker ==="
echo "Source:  ${HOST_SRC_DIR}"
echo "Output:  ${HOST_OUTPUT_DIR}"
echo "CCache:  /build/ccache (inside container)"
echo "Jobs:    ${BUILD_JOBS}"
echo "Arch:    ${APPIMAGE_ARCH}"
echo ""

docker run "${DOCKER_RUN_ARGS[@]}" "${IMAGE_NAME}" \
    bash /src/packaging/appimage/build.sh

# Re-set OUTPUT_DIR to the local path for the results display
OUTPUT_DIR="${HOST_OUTPUT_DIR}"

# ---- Show results ----
echo ""
echo "=== AppImage build complete ==="
echo "Output directory: ${OUTPUT_DIR}"
echo ""
find "${OUTPUT_DIR}" -maxdepth 1 -name "*.AppImage" -exec ls -lh {} \; 2>/dev/null || echo "  (output written to host filesystem at ${OUTPUT_DIR})"
echo ""
