#!/usr/bin/env bash
#
# Build the Mixxx AppImage in the same glibc-2.28 Rocky 8 container as CI.
#
# This one file is used in two ways. CI runs it inside its rockylinux:8
# container, and a normal host runs it, which starts the same container and
# re-runs this file inside, so the steps live in one place and CI exercises
# them on every build. In both cases the script installs the tools, CMake and
# the buildenv.
#
# The paths come from the environment so the same steps work in both layouts.
# MIXXX_SOURCE_DIR is the repository checkout and MIXXX_BUILD_DIR the build
# directory. When the script sets up the container itself, CMake is reused from
# PATH when it is already 4.2 or newer and installed otherwise. The buildenv
# archive is downloaded by CMake into
# BUILDENV_BASEPATH; keep that outside the checkout, because a buildenv inside
# the source tree makes Qt pull its static FFmpeg media plugin into the link
# line.
#
# The build steps are install/check tools, configure, build, ctest, cpack,
# tools/check_appimage_floor_deps.py and a smoke test.

set -euo pipefail
export LANG=C.UTF-8 LC_ALL=C.UTF-8

# ===========================================================================
# Container branch: run inside the rockylinux:8 container.
# ===========================================================================
if [ "${MIXXX_APPIMAGE_IN_CONTAINER:-0}" = "1" ]; then
    MIXXX_SOURCE_DIR="${MIXXX_SOURCE_DIR:-/src}"
    MIXXX_BUILD_DIR="${MIXXX_BUILD_DIR:-/build}"
    MIXXX_CMAKE_DIR="${MIXXX_CMAKE_DIR:-/opt/cmake}"
    BUILDENV_BASEPATH="${BUILDENV_BASEPATH:-/tmp/buildenv}"
    export BUILDENV_BASEPATH

    # Install the tools, CMake and the buildenv unless the caller already
    # provided them (the CI workflow only pre-installs git and python before
    # checkout, so this runs for CI and local builds alike).
    if [ -z "${MIXXX_VCPKG_ROOT:-}" ]; then
        echo "---- [Rocky8] Install base tools ----"
        dnf install -y git python39 curl
        alternatives --set python3 /usr/bin/python3.9

        # The container is root but the checkout belongs to the host user;
        # without this git refuses the dubious ownership dir (e.g. the version
        # describe).
        git config --global --add safe.directory "${MIXXX_SOURCE_DIR}"

        echo "---- [AppImage] Set up cmake ----"
        cmake_is_new_enough() {
            command -v cmake >/dev/null 2>&1 &&
                cmake --version 2>/dev/null |
                    awk -F'[ .]' 'NR==1 {ok=($3>4||($3==4&&$4>=2)); exit !ok}'
        }
        if ! cmake_is_new_enough; then
            # The CPack AppImage generator requires CMake >= 4.2, which dnf
            # does not provide on Rocky 8. Install the pinned release into
            # MIXXX_CMAKE_DIR and put it first on PATH.
            CMAKE_VERSION=4.2.8
            arc=$(uname -m)
            cm_fname="cmake-${CMAKE_VERSION}-linux-${arc}.tar.gz"
            if [ ! -x "${MIXXX_CMAKE_DIR}/v${CMAKE_VERSION}/bin/cmake" ]; then
                curl -fsSL --retry 3 --connect-timeout 15 \
                    -o /tmp/cmake.tar.gz \
                    "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${cm_fname}"
                csha=$(curl -fsSL --retry 3 --connect-timeout 15 \
                    "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-SHA-256.txt" \
                    | awk -v a="${cm_fname}" '$2 == a {print $1}')
                [ -n "${csha}" ] || { echo "ERROR: no SHA-256 entry for ${cm_fname}" >&2; exit 1; }
                echo "${csha}  /tmp/cmake.tar.gz" | sha256sum -c -
                mkdir -p "${MIXXX_CMAKE_DIR}/v${CMAKE_VERSION}"
                tar -xzf /tmp/cmake.tar.gz -C "${MIXXX_CMAKE_DIR}/v${CMAKE_VERSION}" --strip-components=1
                rm /tmp/cmake.tar.gz
            fi
            export PATH="${MIXXX_CMAKE_DIR}/v${CMAKE_VERSION}/bin:${PATH}"
        fi

        echo "---- [macOS/Linux] Set up build environment ----"
        # Source the buildenv setup so its variables land in this shell.
        # GITHUB_ENV must still be set because the script reads it under
        # set -u; pointing it at /dev/null discards the output meant for CI.
        GITHUB_ENV=/dev/null
        BUILDENV_RELEASE=TRUE
        export GITHUB_ENV BUILDENV_RELEASE
        # shellcheck disable=SC1091
        . "${MIXXX_SOURCE_DIR}/tools/appimage_buildenv.sh" setup
        unset GITHUB_ENV BUILDENV_RELEASE

        echo "---- [Rocky8] Config ALSA null device ----"
        printf 'pcm.!default {\n  type null\n}\npcm.sysdefault null\n' > /etc/asound.conf
    fi

    # The Docker wrapper passes HOST_UID and HOST_GID; then hand the files the
    # root container wrote back to the caller on exit (normal, aborted and
    # interactive-shell runs). CI does not set them and would otherwise walk
    # the whole build directory for nothing.
    if [ -n "${HOST_UID:-}" ] && [ -n "${HOST_GID:-}" ]; then
        # shellcheck disable=SC2329
        fix_owner() {
            chown -R "${HOST_UID}:${HOST_GID}" "${MIXXX_BUILD_DIR}" /cache 2>/dev/null || true
        }
        trap 'fix_owner' EXIT
    fi

    if [ "${WRAPPER_MODE:-build}" = shell ]; then
        mkdir -p "${MIXXX_BUILD_DIR}"
        cd "${MIXXX_BUILD_DIR}"
        bash || true # child, so the EXIT trap still chowns after the session
        exit 0
    fi

    echo "---- Configure ----"
    mkdir -p "${MIXXX_BUILD_DIR}"
    cd "${MIXXX_BUILD_DIR}"
    # The compiler is pinned because CMake's automatic detection of
    # gcc-toolset-12 fails inside the rockylinux:8 container ("compiler
    # identification is unknown") while the compiler itself works.  GCC 12
    # emits a false-positive -Warray-bounds in Qt QVariant code, suppressed
    # while keeping -Werror everywhere else.  --allow-shlib-undefined lets a
    # bundled shared library refer to a symbol the host does not provide; the
    # AppImage ships its own copy, which satisfies the reference at runtime.
    cmake -S "${MIXXX_SOURCE_DIR}" -B "${MIXXX_BUILD_DIR}" \
        -DQT6=ON \
        -DQML=ON \
        -DMIXXX_APPIMAGE=ON \
        -DINSTALL_USER_UDEV_RULES=OFF \
        -DCMAKE_C_COMPILER=/opt/rh/gcc-toolset-12/root/usr/bin/cc \
        -DCMAKE_CXX_COMPILER=/opt/rh/gcc-toolset-12/root/usr/bin/c++ \
        -DCMAKE_CXX_FLAGS=-Wno-error=array-bounds \
        -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O2 -DNDEBUG" \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_INSTALL_LIBDIR=lib \
        -DCMAKE_EXE_LINKER_FLAGS="-Wl,--allow-shlib-undefined" \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DWARNINGS_FATAL=ON \
        -DDEBUG_ASSERTIONS_FATAL=OFF \
        -DBATTERY=ON \
        -DBROADCAST=ON \
        -DDOWNLOAD_MANUAL=ON \
        -DHID=ON \
        -DKEYFINDER=ON \
        -DLILV=ON \
        -DOPUS=ON \
        -DQTKEYCHAIN=ON \
        -DVINYLCONTROL=ON

    echo "---- Build ----"
    ccache --zero-stats
    ccache --max-size=2G
    cmake --build "${MIXXX_BUILD_DIR}" --config RelWithDebInfo --parallel "$(nproc)"

    echo "---- ccache stats ----"
    ccache --show-stats
    ccache --cleanup

    echo "---- Test ----"
    GTEST_COLOR=1 CTEST_PARALLEL_LEVEL=3 CTEST_OUTPUT_ON_FAILURE=1 ctest --timeout 45

    echo "---- Package (cpack -G AppImage) ----"
    cpack -G AppImage -V

    echo "---- [AppImage] Check floor dependencies ----"
    # The AppImage delegates the base system stack to the host, so the shipped
    # binary must not reference symbols or symbol versions the glibc 2.28
    # floor cannot provide.  The container has no findutils, so pick the
    # AppImage with a glob.
    APPIMAGE=
    for candidate in "${MIXXX_BUILD_DIR}"/*.AppImage; do
        if [ -e "${candidate}" ]; then
            APPIMAGE=${candidate}
            break
        fi
    done
    if [ -z "${APPIMAGE}" ]; then
        echo "ERROR: no AppImage found in ${MIXXX_BUILD_DIR}" >&2
        exit 1
    fi
    python3 "${MIXXX_SOURCE_DIR}/tools/check_appimage_floor_deps.py" "${APPIMAGE}"

    echo "---- [AppImage] Smoke test ----"
    # Run the packaged AppImage once to catch loader-level failures.  The
    # container has no display, so --version may exit non-zero after
    # initializing Qt; reaching Mixxx startup is what matters.
    set +e
    APPIMAGE_EXTRACT_AND_RUN=1 timeout 60 "${APPIMAGE}" --version >/tmp/smoke.log 2>&1
    rc=$?
    set -e
    if grep -qE "Inconsistency detected by ld.so|error while loading shared libraries|cannot open shared object file" /tmp/smoke.log; then
        echo "FAIL: AppImage failed at loader level (rc=${rc})"
        cat /tmp/smoke.log
        exit 1
    fi
    if ! grep -qE "Mixxx [0-9]|Using preferences|Selected Qt style|ConfigObject:" /tmp/smoke.log; then
        echo "FAIL: AppImage did not reach Mixxx startup (rc=${rc})"
        cat /tmp/smoke.log
        exit 1
    fi
    echo "SMOKE OK"
    echo "AppImage: ${APPIMAGE}"
    exit 0
fi

# ===========================================================================
# Host branch: start the container and re-run this same file inside it.
# ===========================================================================
usage() {
    cat <<'EOF'
Usage (host):
  appimage_build.sh [build|shell] [options]
Options:
  --build-dir DIR      build/output dir (default <repo>/build-appimage)
  --cache-dir DIR      dnf/cmake/ccache/buildenv cache base
                       (default ${XDG_CACHE_HOME:-$HOME/.cache}/mixxx-appimage)
  --proxy URL          HTTP(S)/SOCKS proxy for downloads inside the container
  --help
EOF
}

need_value() {
    if [ $# -lt 2 ]; then
        echo "ERROR: $1 needs a value" >&2
        usage >&2
        exit 2
    fi
}

MIXXX_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

MODE=build
BUILD_DIR=${MIXXX_ROOT}/build-appimage
CACHE_DIR=${XDG_CACHE_HOME:-${HOME}/.cache}/mixxx-appimage
PROXY_URL=
IMAGE=rockylinux:8

while [ $# -gt 0 ]; do
    case "$1" in
        build|shell) MODE=$1 ;;
        --build-dir) need_value "$@"; BUILD_DIR=$2; shift ;;
        --cache-dir) need_value "$@"; CACHE_DIR=$2; shift ;;
        --proxy) need_value "$@"; PROXY_URL=$2; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "ERROR: unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
    shift
done

HOST_ARCH=$(uname -m)
case "${HOST_ARCH}" in
    x86_64|aarch64) ;;
    *) echo "ERROR: unsupported architecture for the AppImage buildenv: ${HOST_ARCH}" >&2; exit 2 ;;
esac

mkdir -p \
    "${BUILD_DIR}" \
    "${CACHE_DIR}/dnf" \
    "${CACHE_DIR}/cmake" \
    "${CACHE_DIR}/ccache" \
    "${CACHE_DIR}/buildenv"

# The daemon resolves bind sources in its own namespace; cd && pwd -P is
# portable and the dirs exist.
BUILD_DIR="$(cd "${BUILD_DIR}" && pwd -P)"
CACHE_DIR="$(cd "${CACHE_DIR}" && pwd -P)"

netproxy_flags=()
if [ -n "${PROXY_URL}" ]; then
    netproxy_flags+=(-e ALL_PROXY="${PROXY_URL}" -e all_proxy="${PROXY_URL}")
    netproxy_flags+=(-e HTTP_PROXY="${PROXY_URL}" -e http_proxy="${PROXY_URL}")
    netproxy_flags+=(-e HTTPS_PROXY="${PROXY_URL}" -e https_proxy="${PROXY_URL}")
else
    for v in HTTP_PROXY http_proxy HTTPS_PROXY https_proxy ALL_PROXY all_proxy; do
        [ -n "${!v:-}" ] && netproxy_flags+=(-e "${v}=${!v}")
    done
fi

# Docker silently creates empty dirs for bind sources it cannot see, so probe
# for a file in the repository and print docker's own error when it fails.
probe_status=0
probe_output=$(docker run --rm ${netproxy_flags[@]+"${netproxy_flags[@]}"} \
        -v "${MIXXX_ROOT}:/p/src:ro" \
        -v "${BUILD_DIR}:/p/build:ro" \
        -v "${CACHE_DIR}:/p/cache:ro" \
        "${IMAGE}" bash -c 'test -f /p/src/CMakeLists.txt -a -d /p/build -a -d /p/cache' \
        2>&1) || probe_status=$?
if [ "${probe_status}" -ne 0 ]; then
    [ -n "${probe_output}" ] && printf '%s\n' "${probe_output}" >&2
    echo "ERROR: docker cannot mount ${MIXXX_ROOT}, ${BUILD_DIR} and ${CACHE_DIR} into ${IMAGE}." >&2
    exit 2
fi

run_flags=(
    --rm
    --add-host host.docker.internal:host-gateway
    -v "${MIXXX_ROOT}:/src"
    -v "${BUILD_DIR}:/build"
    -v "${CACHE_DIR}/dnf:/var/cache/dnf"
    -v "${CACHE_DIR}/cmake:/opt/cmake"
    -v "${CACHE_DIR}/ccache:/cache/ccache"
    -v "${CACHE_DIR}/buildenv:/cache/buildenv"
    -e MIXXX_SOURCE_DIR=/src
    -e MIXXX_BUILD_DIR=/build
    -e MIXXX_CMAKE_DIR=/opt/cmake
    -e CCACHE_DIR=/cache/ccache
    -e BUILDENV_BASEPATH=/cache/buildenv
    -e HOST_UID="$(id -u)"
    -e HOST_GID="$(id -g)"
    -e MIXXX_APPIMAGE_IN_CONTAINER=1
    -e "WRAPPER_MODE=${MODE}"
)
run_flags+=(${netproxy_flags[@]+"${netproxy_flags[@]}"})

echo "== Running ${IMAGE} (mode: ${MODE}) =="
echo "   sources: ${MIXXX_ROOT} (writable, like CI's checkout)"
echo "   build  : ${BUILD_DIR}"
echo "   cache  : ${CACHE_DIR}"
echo "   proxy  : ${PROXY_URL:-<none>}"

if [ "${MODE}" = shell ]; then
    docker run -it "${run_flags[@]}" "${IMAGE}" bash /src/tools/appimage_build.sh
else
    docker run -i "${run_flags[@]}" "${IMAGE}" bash /src/tools/appimage_build.sh
fi
