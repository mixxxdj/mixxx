#!/usr/bin/env python3
"""
Build Mixxx against multiple Qt versions (Debian 13.2) with separate Qt prefixes,
including QtHttpServer and on-the-fly TLS keypair validation.

Adjusted behavior per request:
  - ALL downloads + build working dirs default to /opt/mixxx-multiqt/*
  - Qt installs default to /opt/qt/<version>
  - Mixxx source path is still user-specified (--mixxx-src) and is NOT forced under /opt

Default Mixxx repo:
  https://github.com/mccartyp/mixxx.git
"""

from __future__ import annotations

import argparse
import dataclasses
import os
import shutil
import subprocess
import sys
import tarfile
import textwrap
import time
import urllib.request
from pathlib import Path


@dataclasses.dataclass(frozen=True)
class QtVersion:
    version: str
    stream: str


QT_CATALOG: dict[str, QtVersion] = {
    "6.8.3": QtVersion(version="6.8.3", stream="6.8"),
    "6.10.1": QtVersion(version="6.10.1", stream="6.10"),
}

DEFAULT_QT_VERSIONS = ["6.8.3", "6.10.1"]

DEFAULT_OPT_ROOT = Path("/opt/mixxx-multiqt")  # downloads + builds + validation live here by default
DEFAULT_INSTALL_ROOT = Path("/opt")            # Qt installs live under /opt/qt/<ver> by default


def run(cmd: list[str], *, cwd: Path | None = None, env: dict[str, str] | None = None) -> None:
    print(f"\n[RUN] {' '.join(cmd)}")
    subprocess.run(cmd, cwd=str(cwd) if cwd else None, env=env, check=True)


def capture(cmd: list[str], *, cwd: Path | None = None, env: dict[str, str] | None = None) -> str:
    print(f"\n[CAPTURE] {' '.join(cmd)}")
    out = subprocess.check_output(cmd, cwd=str(cwd) if cwd else None, env=env)
    return out.decode("utf-8", errors="replace").strip()


def download(url: str, dest: Path) -> None:
    dest.parent.mkdir(parents=True, exist_ok=True)
    if dest.exists() and dest.stat().st_size > 0:
        print(f"[SKIP] Already downloaded: {dest}")
        return
    print(f"[DL] {url}\n     -> {dest}")
    with urllib.request.urlopen(url) as r, dest.open("wb") as f:
        shutil.copyfileobj(r, f)


def extract_tar_xz(archive: Path, dest_dir: Path) -> Path:
    """
    Extract tar.xz into dest_dir; return extracted top-level directory.
    Uses a marker file to avoid re-extracting.
    """
    dest_dir.mkdir(parents=True, exist_ok=True)
    marker = dest_dir / f".extracted-{archive.name}"
    if marker.exists():
        subdirs = [p for p in dest_dir.iterdir() if p.is_dir()]
        if len(subdirs) == 1:
            return subdirs[0]
        for p in subdirs:
            if p.name.startswith("qt") or "httpserver" in p.name:
                return p
        return dest_dir

    print(f"[EXTRACT] {archive} -> {dest_dir}")
    with tarfile.open(archive, mode="r:xz") as tf:
        tf.extractall(dest_dir)
    marker.write_text(time.strftime("%Y-%m-%d %H:%M:%S\n"))

    subdirs = [p for p in dest_dir.iterdir() if p.is_dir()]
    if len(subdirs) == 1:
        return subdirs[0]
    return dest_dir


def apt_install_deps() -> None:
    """
    Best-effort dependency install for:
      - Building Qt from source
      - Building Mixxx
      - TLS keypair generation
    """
    pkgs = [
        "build-essential", "ninja-build", "cmake", "git", "pkg-config",
        "python3", "python3-pip", "python3-venv",
        "perl", "gperf", "bison", "flex", "nodejs",
        "openssl",
        "libssl-dev", "zlib1g-dev", "libzstd-dev", "libbrotli-dev",
        "libdbus-1-dev", "libglib2.0-dev",
        "libx11-dev", "libx11-xcb-dev", "libxext-dev", "libxfixes-dev",
        "libxi-dev", "libxrender-dev", "libxrandr-dev", "libxkbcommon-dev",
        "libxkbcommon-x11-dev",
        "libxcb1-dev", "libxcb-glx0-dev", "libxcb-keysyms1-dev",
        "libxcb-image0-dev", "libxcb-shm0-dev", "libxcb-icccm4-dev",
        "libxcb-sync-dev", "libxcb-xfixes0-dev", "libxcb-shape0-dev",
        "libxcb-randr0-dev", "libxcb-render-util0-dev", "libxcb-util-dev",
        "libxcb-xinerama0-dev", "libxcb-xkb-dev",
        "libgl1-mesa-dev", "libegl1-mesa-dev",
        "libfontconfig1-dev", "libfreetype6-dev", "libharfbuzz-dev",
        "libpng-dev", "libjpeg-dev", "libicu-dev",
        # Mixxx deps (best-effort)
        "libasound2-dev",
        "libid3tag0-dev", "libtag1-dev",
        "libmad0-dev", "libvorbis-dev", "libogg-dev", "libflac-dev",
        "libsndfile1-dev",
        "libchromaprint-dev", "libfftw3-dev",
        "libprotobuf-dev", "protobuf-compiler",
        "libsqlite3-dev",
        "librubberband-dev",
        "libshout3-dev",
        "libusb-1.0-0-dev",
        "portaudio19-dev",
        "libupower-glib-dev",
        "libkeyfinder-dev",
        "libhidapi-dev",
        # Qt multimedia backends
        "libgstreamer1.0-dev",
        "libgstreamer-plugins-base1.0-dev",
        "libgstreamer-plugins-good1.0-dev",
        "libgstreamer-plugins-bad1.0-dev",
        # Audio backends
        "libpulse-dev",
        # FFmpeg for Qt multimedia and Mixxx
        "libavcodec-dev",
        "libavformat-dev",
        "libavdevice-dev",
        "libavutil-dev",
        "libavfilter-dev",
        "libswscale-dev",
        "libswresample-dev",
        "libva-dev",
        # Testing and benchmarking
        "libgtest-dev",
        "googletest",
        "libbenchmark-dev",
    ]
    run(["sudo", "apt-get", "update"])
    run(["sudo", "apt-get", "install", "-y"] + pkgs)
    try:
        run(["sudo", "apt-get", "build-dep", "-y", "mixxx"])
    except subprocess.CalledProcessError:
        print("[WARN] 'apt-get build-dep mixxx' failed (maybe src repos not enabled). Continuing.")


def ensure_openssl_dev_present() -> None:
    """Fail fast with a clear message when OpenSSL headers/libs are missing."""
    required_paths = [
        Path("/usr/include/openssl/asn1.h"),
        Path("/usr/lib/x86_64-linux-gnu/libssl.so"),
        Path("/usr/lib/x86_64-linux-gnu/libcrypto.so"),
    ]

    missing = [str(p) for p in required_paths if not p.exists()]
    if missing:
        missing_list = "\n  - " + "\n  - ".join(missing)
        raise SystemExit(
            "Missing OpenSSL development files required to build Qt:"\
            f"{missing_list}\n"
            "Install libssl-dev (or rerun with --install-deps) and try again."
        )


def qt_urls(qt: QtVersion) -> tuple[str, str]:
    qt_everywhere = (
        f"https://download.qt.io/official_releases/qt/{qt.stream}/{qt.version}/single/"
        f"qt-everywhere-src-{qt.version}.tar.xz"
    )
    qthttpserver = (
        f"https://download.qt.io/official_releases/qt/{qt.stream}/{qt.version}/submodules/"
        f"qthttpserver-everywhere-src-{qt.version}.tar.xz"
    )
    return qt_everywhere, qthttpserver


def configure_env_for_qt(prefix: Path) -> dict[str, str]:
    env = os.environ.copy()
    env["PATH"] = f"{prefix / 'bin'}:{env.get('PATH', '')}"
    env["CMAKE_PREFIX_PATH"] = str(prefix)
    env["PKG_CONFIG_PATH"] = f"{prefix / 'lib' / 'pkgconfig'}:{env.get('PKG_CONFIG_PATH','')}"
    env["LD_LIBRARY_PATH"] = f"{prefix / 'lib'}:{env.get('LD_LIBRARY_PATH','')}"
    return env


def build_qt(
    qt: QtVersion,
    *,
    opt_root: Path,
    install_root: Path,
    jobs: int,
    keep_build_dirs: bool,
    force_rebuild: bool,
) -> Path:
    """
    Build Qt from source into install_root/qt/<ver> and return the prefix.
    All download/build staging happens under opt_root.
    """
    ensure_openssl_dev_present()

    prefix = install_root / "qt" / qt.version
    qt_bin = prefix / "bin" / "qmake6"
    if qt_bin.exists() and not force_rebuild:
        print(f"[SKIP] Qt {qt.version} appears installed at {prefix}")
        return prefix

    prefix.mkdir(parents=True, exist_ok=True)

    src_cache = opt_root / "src_cache"
    build_root = opt_root / "build"
    src_root = opt_root / "src"

    qt_url, http_url = qt_urls(qt)
    qt_tar = src_cache / f"qt-everywhere-src-{qt.version}.tar.xz"
    http_tar = src_cache / f"qthttpserver-everywhere-src-{qt.version}.tar.xz"

    download(qt_url, qt_tar)
    download(http_url, http_tar)

    qt_src_dir = extract_tar_xz(qt_tar, src_root / f"qt-everywhere-src-{qt.version}")
    http_src_dir = extract_tar_xz(http_tar, src_root / f"qthttpserver-src-{qt.version}")

    qt_build_dir = build_root / f"qt-{qt.version}"
    # Always clean build dir to avoid cached failures (especially OpenSSL detection)
    if qt_build_dir.exists():
        print(f"[CLEAN] Removing existing build dir: {qt_build_dir}")
        shutil.rmtree(qt_build_dir, ignore_errors=True)
    qt_build_dir.mkdir(parents=True, exist_ok=True)

    configure_cmd = [
        str(qt_src_dir / "configure"),
        "-prefix", str(prefix),
        "-opensource", "-confirm-license",
        "-release",
        "-nomake", "examples",
        "-nomake", "tests",
        "-cmake-generator", "Ninja",

        # Core build options - use runtime OpenSSL loading to avoid detection issues
        "-openssl-runtime",
        "-qt-zlib",
        "-qt-libpng",
        "-qt-libjpeg",

        # --- KEEP QML / QUICK STACK (REQUIRED BY MIXXX) ---
        # qtdeclarative
        # qtshadertools
        # qtquickcontrols2
        # qtquicktimeline
        # qt5compat
        # qtmultimedia

        # --- SAFE SKIPS ---
        "-skip", "qtscxml",
        "-skip", "qtopcua",
        "-skip", "qtvirtualkeyboard",

        "-skip", "qttranslations",
        "-skip", "qttools",
        "-skip", "qtdoc",

        "-skip", "qtwebengine",
        "-skip", "qtwebview",

#        "-skip", "qt3d",
#        "-skip", "qtquick3d",
#        "-skip", "qtquick3dphysics",
#        "-skip", "qtquick3dparticles",
#        "-skip", "qtquickeffectmaker",

        "-skip", "qtsensors",
        "-skip", "qtlocation",
        "-skip", "qtlottie",
        "-skip", "qtconnectivity",
        "-skip", "qtspeech",
        "-skip", "qtremoteobjects",
        "-skip", "qtmqtt",
        "-skip", "qtgrpc",
        "-skip", "qtcoap",

        # Pass CMake variables to force OpenSSL detection
        # Qt's detection tests fail even with correct installation, so we override them
        "--",
        "-DOPENSSL_ROOT_DIR=/usr",
        "-DOPENSSL_INCLUDE_DIR=/usr/include",
        "-DOPENSSL_CRYPTO_LIBRARY=/usr/lib/x86_64-linux-gnu/libcrypto.so",
        "-DOPENSSL_SSL_LIBRARY=/usr/lib/x86_64-linux-gnu/libssl.so",
        "-DOpenSSL_FOUND=TRUE",
        "-DOPENSSL_FOUND=TRUE",
        "-DOPENSSL_VERSION=3.0.13",
        "-DQT_FEATURE_openssl=ON",
        "-DQT_FEATURE_openssl_runtime=ON",
        "-DTEST_opensslv30_headers=TRUE",
    ]

    # Set up environment for Qt configure
    configure_env = os.environ.copy()
    configure_env["PKG_CONFIG_PATH"] = f"/usr/lib/x86_64-linux-gnu/pkgconfig:{configure_env.get('PKG_CONFIG_PATH', '')}"

    run(configure_cmd, cwd=qt_build_dir, env=configure_env)
    run(["cmake", "--build", ".", f"-j{jobs}"], cwd=qt_build_dir, env=configure_env)
    run(["cmake", "--install", "."], cwd=qt_build_dir, env=configure_env)

    http_build_dir = build_root / f"qthttpserver-{qt.version}"
    if force_rebuild and http_build_dir.exists():
        shutil.rmtree(http_build_dir, ignore_errors=True)
    http_build_dir.mkdir(parents=True, exist_ok=True)

    env = configure_env_for_qt(prefix)
    run(
        [
            "cmake", "-S", str(http_src_dir), "-B", str(http_build_dir),
            "-G", "Ninja",
            f"-DCMAKE_INSTALL_PREFIX={prefix}",
            "-DCMAKE_BUILD_TYPE=Release",
        ],
        env=env,
    )
    run(["cmake", "--build", str(http_build_dir), f"-j{jobs}"], env=env)
    run(["cmake", "--install", str(http_build_dir)], env=env)

    if not keep_build_dirs:
        shutil.rmtree(qt_build_dir, ignore_errors=True)
        shutil.rmtree(http_build_dir, ignore_errors=True)

    return prefix


def write_tls_validation_project(dest: Path) -> None:
    dest.mkdir(parents=True, exist_ok=True)
    (dest / "CMakeLists.txt").write_text(textwrap.dedent("""\
        cmake_minimum_required(VERSION 3.21)
        project(qt_tls_httpserver_validate LANGUAGES CXX)

        set(CMAKE_CXX_STANDARD 17)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)

        find_package(Qt6 REQUIRED COMPONENTS Core Network HttpServer)

        add_executable(qt_tls_httpserver_validate main.cpp)
        target_link_libraries(qt_tls_httpserver_validate PRIVATE Qt6::Core Qt6::Network Qt6::HttpServer)
    """))

    (dest / "main.cpp").write_text(textwrap.dedent(r"""\
        #include <QCoreApplication>
        #include <QDebug>
        #include <QFile>
        #include <QSslSocket>
        #include <QSslServer>
        #include <QSslKey>
        #include <QSslCertificate>
        #include <QSslConfiguration>

        #include <QHttpServer>
        #include <QHttpServerResponse>

        #include <type_traits>
        #include <utility>

        static QByteArray readFile(const QString& p) {
            QFile f(p);
            if (!f.open(QIODevice::ReadOnly)) return {};
            return f.readAll();
        }

        template <typename T, typename = void>
        struct BindResultHasBoolConversion : std::false_type {};

        template <typename T>
        struct BindResultHasBoolConversion<
                T,
                std::void_t<decltype(static_cast<bool>(std::declval<const T&>()))>>
                : std::true_type {};

        template <typename Result, typename Enable = void>
        struct BindInvoker;

        template <typename Result>
        struct BindInvoker<Result, std::enable_if_t<std::is_void_v<Result>>> {
            static bool bind(QHttpServer* http, QSslServer* server) {
                http->bind(server);
                return true;
            }
        };

        template <typename Result>
        struct BindInvoker<Result, std::enable_if_t<!std::is_void_v<Result>>> {
            template <typename T>
            static bool evaluateBindResult(T&& result) {
                using Decayed = std::decay_t<T>;
                if constexpr (std::is_convertible_v<Decayed, bool>) {
                    return static_cast<bool>(result);
                } else if constexpr (BindResultHasBoolConversion<Decayed>::value) {
                    return static_cast<bool>(result);
                } else {
                    return true; // Fallback for status-like types without bool conversion
                }
            }

            static bool bind(QHttpServer* http, QSslServer* server) {
                Result result = http->bind(server); // Perform the bind exactly once.
                return evaluateBindResult(result);
            }
        };

        int main(int argc, char** argv) {
            QCoreApplication app(argc, argv);

            qInfo() << "Qt version:" << qVersion();
            qInfo() << "QSslSocket supports SSL:" << QSslSocket::supportsSsl();

            if (!QSslSocket::supportsSsl()) {
                qCritical() << "FAIL: QtNetwork reports no TLS support (QSslSocket::supportsSsl() == false).";
                return 2;
            }

            const QString certPath = qEnvironmentVariable("QT_TLS_CERT_PEM");
            const QString keyPath  = qEnvironmentVariable("QT_TLS_KEY_PEM");
            if (certPath.isEmpty() || keyPath.isEmpty()) {
                qCritical() << "FAIL: QT_TLS_CERT_PEM / QT_TLS_KEY_PEM not set.";
                return 3;
            }

            const QByteArray certPem = readFile(certPath);
            const QByteArray keyPem  = readFile(keyPath);

            const QSslCertificate cert(certPem, QSsl::Pem);
            const QSslKey key(keyPem, QSsl::Rsa, QSsl::Pem);

            if (cert.isNull() || key.isNull()) {
                qCritical() << "FAIL: Provided cert/key could not be loaded:" << certPath << keyPath;
                return 4;
            }

            QSslConfiguration cfg = QSslConfiguration::defaultConfiguration();
            cfg.setLocalCertificate(cert);
            cfg.setPrivateKey(key);
            cfg.setPeerVerifyMode(QSslSocket::VerifyNone);

            QHttpServer http;
            http.route("/", []() { return QHttpServerResponse("text/plain", "ok\n"); });

            QSslServer sslServer;
            sslServer.setSslConfiguration(cfg);

            if (!sslServer.listen(QHostAddress::LocalHost, 0)) {
                qCritical() << "FAIL: QSslServer could not listen:" << sslServer.errorString();
                return 5;
            }

            const quint16 port = sslServer.serverPort();

            // QtHttpServer::bind has returned different types across Qt 6.x releases
            // (e.g. bool in earlier versions, void or status-like types in newer ones).
            // Keep this compatible with C++17 (no requires/constraints) while handling
            // the varying signatures by probing for bool-convertible results and
            // otherwise assuming success.
            using BindResult = decltype(std::declval<QHttpServer>().bind(std::declval<QSslServer*>()));

            if (!BindInvoker<BindResult>::bind(&http, &sslServer)) {
                qCritical() << "FAIL: QHttpServer could not bind to QSslServer.";
                return 6;
            }

            qInfo() << "PASS: TLS+QtHttpServer validated. Listening on https://127.0.0.1:" << port;
            return 0;
        }
    """))


def generate_ephemeral_tls_keypair(out_dir: Path, *, common_name: str, days: int = 1) -> tuple[Path, Path]:
    out_dir.mkdir(parents=True, exist_ok=True)
    cert_pem = out_dir / "cert.pem"
    key_pem = out_dir / "key.pem"

    for p in (cert_pem, key_pem):
        try:
            p.unlink()
        except FileNotFoundError:
            pass

    subj = f"/CN={common_name}"
    run([
        "openssl", "req", "-x509", "-newkey", "rsa:2048",
        "-keyout", str(key_pem),
        "-out", str(cert_pem),
        "-sha256",
        "-days", str(days),
        "-nodes",
        "-subj", subj,
    ])
    return cert_pem, key_pem


def validate_qt_tls_httpserver(prefix: Path, *, opt_root: Path, jobs: int) -> None:
    proj = opt_root / "validate" / f"qt-tls-httpserver-{prefix.name}"
    write_tls_validation_project(proj)

    tls_dir = proj / "tls"
    cert_pem, key_pem = generate_ephemeral_tls_keypair(
        tls_dir,
        common_name=f"qt-tls-test-{prefix.name}",
        days=1,
    )

    build_dir = proj / "build"
    build_dir.mkdir(parents=True, exist_ok=True)

    env = configure_env_for_qt(prefix)
    env["QT_TLS_CERT_PEM"] = str(cert_pem)
    env["QT_TLS_KEY_PEM"] = str(key_pem)

    run(["cmake", "-S", str(proj), "-B", str(build_dir), "-G", "Ninja",
         "-DCMAKE_BUILD_TYPE=Release"], env=env)
    run(["cmake", "--build", str(build_dir), f"-j{jobs}"], env=env)

    exe = build_dir / "qt_tls_httpserver_validate"
    run([str(exe)], env=env)


def ensure_mixxx_source(mixxx_src: Path, *, git_url: str, update: bool) -> Path:
    if not mixxx_src.exists():
        mixxx_src.parent.mkdir(parents=True, exist_ok=True)
        run(["git", "clone", "--recursive", git_url, str(mixxx_src)])
        return mixxx_src

    if not (mixxx_src / "CMakeLists.txt").exists():
        raise FileNotFoundError(f"{mixxx_src} exists but does not look like a Mixxx source tree (no CMakeLists.txt).")

    if update:
        run(["git", "fetch", "--all", "--prune"], cwd=mixxx_src)
        try:
            run(["git", "pull", "--ff-only"], cwd=mixxx_src)
        except subprocess.CalledProcessError:
            print("[WARN] git pull --ff-only failed (local changes or divergent). Continuing with current checkout.")
        run(["git", "submodule", "update", "--init", "--recursive"], cwd=mixxx_src)

    return mixxx_src


def build_mixxx_against_qt(
    qt_prefix: Path,
    mixxx_src: Path,
    *,
    opt_root: Path,
    artifacts_dir: Path,
    jobs: int,
    force_rebuild: bool,
) -> None:
    env = configure_env_for_qt(qt_prefix)

    build_dir = opt_root / "mixxx_build" / f"mixxx-qt-{qt_prefix.name}"
    if force_rebuild and build_dir.exists():
        shutil.rmtree(build_dir, ignore_errors=True)
    build_dir.mkdir(parents=True, exist_ok=True)

    cmake_args = [
        "cmake", "-S", str(mixxx_src), "-B", str(build_dir),
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=RelWithDebInfo",
        f"-DCMAKE_PREFIX_PATH={qt_prefix}",
        "-DMIXXX_BUILD_TESTING=ON",
    ]

    run(cmake_args, env=env)
    run(["cmake", "--build", str(build_dir), f"-j{jobs}"], env=env)

    candidates = [
        build_dir / "mixxx",
        build_dir / "src" / "mixxx",
        build_dir / "mixxx-test",
        build_dir / "src" / "test" / "mixxx-test",
        build_dir / "test" / "mixxx-test",
    ]
    found_mixxx = next((p for p in candidates if p.name == "mixxx" and p.exists() and os.access(p, os.X_OK)), None)
    found_test = next((p for p in candidates if p.name == "mixxx-test" and p.exists() and os.access(p, os.X_OK)), None)

    out_dir = artifacts_dir / f"mixxx-qt-{qt_prefix.name}"
    out_dir.mkdir(parents=True, exist_ok=True)

    if not found_mixxx:
        raise FileNotFoundError(f"Could not find built 'mixxx' binary under {build_dir}")

    shutil.copy2(found_mixxx, out_dir / "mixxx")
    if found_test:
        shutil.copy2(found_test, out_dir / "mixxx-test")
    else:
        print("[WARN] 'mixxx-test' not found. Your branch/config may not produce it or path differs.")

    (out_dir / "provenance.txt").write_text(
        f"Qt prefix: {qt_prefix}\n"
        f"Mixxx src: {mixxx_src}\n"
        f"Mixxx git: {capture(['git','rev-parse','HEAD'], cwd=mixxx_src)}\n"
        f"Built at: {time.strftime('%Y-%m-%d %H:%M:%S')}\n"
        f"Host: {capture(['uname','-a'])}\n"
    )

    print(f"[OK] Artifacts written to: {out_dir}")


def parse_qt_versions(arg_list: list[str] | None) -> list[QtVersion]:
    versions = DEFAULT_QT_VERSIONS if not arg_list else arg_list
    resolved: list[QtVersion] = []
    for v in versions:
        if v not in QT_CATALOG:
            raise ValueError(f"Unsupported Qt version '{v}'. Supported: {', '.join(QT_CATALOG.keys())}")
        resolved.append(QT_CATALOG[v])
    return resolved


def ensure_opt_dirs(opt_root: Path) -> None:
    """
    Create /opt staging directories. If user isn't root, they likely need:
      sudo mkdir -p /opt/mixxx-multiqt && sudo chown $USER:$USER /opt/mixxx-multiqt
    """
    opt_root.mkdir(parents=True, exist_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="Build Qt (multiple versions) + QtHttpServer(TLS) and build Mixxx against each Qt prefix.",
        epilog=textwrap.dedent(f"""\
            Defaults:
              - Staging (downloads/build/validation): {DEFAULT_OPT_ROOT}
              - Qt install prefixes:               {DEFAULT_INSTALL_ROOT}/qt/<version>
              - Mixxx git:                         https://github.com/mccartyp/mixxx.git

            Examples:

              # First-time prep: ensure /opt staging is writable for your user
              sudo mkdir -p {DEFAULT_OPT_ROOT}
              sudo chown $USER:$USER {DEFAULT_OPT_ROOT}

              # Full pipeline (deps + Qt + validate TLS + Mixxx) for default Qt versions
              ./build_mixxx_multiqt.py --install-deps --mixxx-src ~/src/mixxx

              # Only Qt toolchains (skip Mixxx)
              ./build_mixxx_multiqt.py --qt-only --mixxx-src ~/src/mixxx

              # Only Mixxx against already-installed Qt prefixes (skip Qt build)
              ./build_mixxx_multiqt.py --mixxx-only --mixxx-src ~/src/mixxx

              # Select versions
              ./build_mixxx_multiqt.py --qt-versions 6.10.1 --mixxx-src ~/src/mixxx
        """),
    )

    # Mixxx source is user-controlled
    parser.add_argument("--mixxx-src", type=Path, required=True,
                        help="Path to Mixxx source tree (will clone if missing).")
    parser.add_argument("--mixxx-git", type=str, default="https://github.com/mccartyp/mixxx.git",
                        help="Mixxx git URL if cloning.")
    parser.add_argument("--mixxx-update", action="store_true",
                        help="If Mixxx source exists, fetch/pull and update submodules.")

    # /opt staging and installs
    parser.add_argument("--opt-root", type=Path, default=DEFAULT_OPT_ROOT,
                        help=f"Staging root for downloads/builds/validation (default: {DEFAULT_OPT_ROOT}).")
    parser.add_argument("--install-root", type=Path, default=DEFAULT_INSTALL_ROOT,
                        help=f"Install root for Qt prefixes (default: {DEFAULT_INSTALL_ROOT}).")
    parser.add_argument("--artifacts-dir", type=Path, default=DEFAULT_OPT_ROOT / "artifacts",
                        help=f"Where to store per-Qt Mixxx binaries (default: {DEFAULT_OPT_ROOT / 'artifacts'}).")

    # Versions
    parser.add_argument("--qt-versions", nargs="+", default=None,
                        help=f"Qt versions to build/test (default: {' '.join(DEFAULT_QT_VERSIONS)}). "
                             f"Supported: {', '.join(QT_CATALOG.keys())}")

    # Pipeline controls
    parser.add_argument("--install-deps", action="store_true",
                        help="Install apt dependencies (requires sudo).")
    parser.add_argument("--qt-only", action="store_true",
                        help="Only build/install Qt + QtHttpServer + TLS validate (skip Mixxx build).")
    parser.add_argument("--mixxx-only", action="store_true",
                        help="Only build Mixxx against Qt prefixes (skip Qt build).")
    parser.add_argument("--skip-validate", action="store_true",
                        help="Skip TLS/QtHttpServer validation step.")
    parser.add_argument("--force-rebuild", action="store_true",
                        help="Delete and rebuild intermediate build dirs for Qt/Mixxx steps.")
    parser.add_argument("--keep-build-dirs", action="store_true",
                        help="Do not delete intermediate build directories after successful builds.")
    parser.add_argument("--jobs", type=int, default=min(2, os.cpu_count() or 4),
                        help="Parallel build jobs.")

    args = parser.parse_args()

    if args.qt_only and args.mixxx_only:
        print("[ERROR] --qt-only and --mixxx-only are mutually exclusive.", file=sys.stderr)
        return 2

    if args.install_deps:
        apt_install_deps()

    ensure_opt_dirs(args.opt_root)
    args.artifacts_dir.mkdir(parents=True, exist_ok=True)

    qt_versions = parse_qt_versions(args.qt_versions)
    mixxx_src = ensure_mixxx_source(args.mixxx_src, git_url=args.mixxx_git, update=args.mixxx_update)

    for qt in qt_versions:
        print("\n" + "=" * 100)
        print(f"[Qt] Target: Qt {qt.version} (stream {qt.stream})")
        print("=" * 100)

        qt_prefix = args.install_root / "qt" / qt.version

        if not args.mixxx_only:
            qt_prefix = build_qt(
                qt,
                opt_root=args.opt_root,
                install_root=args.install_root,
                jobs=args.jobs,
                keep_build_dirs=args.keep_build_dirs,
                force_rebuild=args.force_rebuild,
            )

        if not args.skip_validate and not args.mixxx_only:
            validate_qt_tls_httpserver(qt_prefix, opt_root=args.opt_root, jobs=args.jobs)

        if not args.qt_only:
            if not qt_prefix.exists():
                raise FileNotFoundError(
                    f"Qt prefix does not exist: {qt_prefix}. Run without --mixxx-only or install Qt at that path."
                )
            if not args.skip_validate and args.mixxx_only:
                validate_qt_tls_httpserver(qt_prefix, opt_root=args.opt_root, jobs=args.jobs)

            build_mixxx_against_qt(
                qt_prefix,
                mixxx_src,
                opt_root=args.opt_root,
                artifacts_dir=args.artifacts_dir,
                jobs=args.jobs,
                force_rebuild=args.force_rebuild,
            )

    print("\n[DONE] Completed requested pipeline across selected Qt versions.")
    print(f"Artifacts: {args.artifacts_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
