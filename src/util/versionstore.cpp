#include "util/versionstore.h"

#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QtGlobal>
#include <utility>

#ifdef __BROADCAST__
#include <shoutidjc/shout.h>
#endif

#ifdef __RUBBERBAND__
#include <rubberband/RubberBandStretcher.h>
#endif

#include <FLAC/format.h>
#include <chromaprint.h>
#include <ebur128.h>
#include <lame/lame.h>
#include <portaudio.h>
#include <sndfile.h>
#include <soundtouch/SoundTouch.h>
#include <taglib.h>
#include <vorbis/codec.h>

#include "util/gitinfostore.h"
#include "version.h"

// https://stackoverflow.com/questions/240353/convert-a-preprocessor-token-to-a-string#comment84146590_240370
#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)

namespace {

const QVersionNumber kMixxxVersionNumber = QVersionNumber(
        MIXXX_VERSION_MAJOR, MIXXX_VERSION_MINOR, MIXXX_VERSION_PATCH);
const QString kMixxxVersionSuffix = QStringLiteral(MIXXX_VERSION_SUFFIX);
const QString kMixxx = QStringLiteral("Mixxx");
const QString kBuildFlags = QStringLiteral(MIXXX_BUILD_FLAGS);

QString ebur128Version() {
    int major = 0;
    int minor = 0;
    int patch = 0;
    ebur128_get_version(&major, &minor, &patch);
    return QStringLiteral("%1.%2.%3")
            .arg(QString::number(major),
                    QString::number(minor),
                    QString::number(patch));
}

} // namespace

// static
QString VersionStore::version() {
    if (kMixxxVersionSuffix.isEmpty()) {
        return kMixxxVersionNumber.toString();
    } else {
        return kMixxxVersionNumber.toString() + QStringLiteral("-") + kMixxxVersionSuffix;
    }
}

// static
QVersionNumber VersionStore::versionNumber() {
    return kMixxxVersionNumber;
}

// static
QString VersionStore::versionSuffix() {
    return kMixxxVersionSuffix;
}

QDateTime VersionStore::date() {
    return QDateTime::fromString(GitInfoStore::date(), Qt::ISODate);
}

// static
QString VersionStore::applicationName() {
    return kMixxx;
}

// MSVC doesn't properly evaluate #if in macro arguments (such as QStringLiteral)
// So I work around that using these #defines.
// static
#ifdef Q_OS_IOS
#define OS_VERSION_STR "iOS"
#elif defined(Q_OS_MACOS)
#define OS_VERSION_STR "macOS"
#elif defined(__LINUX__)
#define OS_VERSION_STR "Linux"
#elif defined(__WINDOWS__)
#define OS_VERSION_STR "Windows"
// Mixxx's CMakeLists.txt does not define this, but FreeBSD's ports system does.
#elif defined(__FREEBSD__)
#define OS_VERSION_STR "FreeBSD"
#elif defined(__BSD__)
#define OS_VERSION_STR "BSD"
#elif defined(__EMSCRIPTEN__)
#define OS_VERSION_STR "Emscripten"
#else
#define OS_VERSION_STR "Unknown OS"
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || \
        defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64) || \
        defined(AMD64) || defined(EM64T) || defined(x86_64)
#define PLATFORM_STR "x86_64"
#elif defined(i386) || defined(__i386) || defined(__i386__) ||     \
        defined(__IA32__) || defined(_M_IX86) || defined(_X86_) || \
        defined(__X86__) || defined(__I86__)
#define PLATFORM_STR "x86"
#elif defined(IA64)
#define PLATFORM_STR "IA64"
#elif defined(__aarch64__) || defined(ARM64)
#define PLATFORM_STR "ARM64"
#elif defined(__arm__) || defined(__thumb__) || defined(_ARM) || \
        defined(_M_ARM) || defined(_M_ARMT) || defined(__arm)
#define PLATFORM_STR "ARM"
#elif defined(mips) || defined(__mips)
#define PLATFORM_STR "MIPS"
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || \
        defined(__ppc__) || defined(__ppc) || defined(__PPC__) ||             \
        defined(__PPC64__) || defined(_ARCH_PPC) || defined(_ARCH_PPC64) ||   \
        defined(_M_PPC)
#define PLATFORM_STR "PowerPC"
#elif defined(__wasm32__)
#define PLATFORM_STR "Wasm32"
#elif defined(__wasm__)
#define PLATFORM_STR "Wasm"
#endif

QString VersionStore::platform() {
    return QStringLiteral(OS_VERSION_STR " " PLATFORM_STR);
}

// static
QString VersionStore::gitBranch() {
    return GitInfoStore::branch();
}

// static
QString VersionStore::gitDescribe() {
    return GitInfoStore::describe();
}

// static
QString VersionStore::gitVersion() {
    QString gitVersion = GitInfoStore::describe();
    if (gitVersion.isEmpty()) {
        gitVersion = QStringLiteral("unknown");
    }

    QString gitBranch = VersionStore::gitBranch();
    if (!gitBranch.isEmpty() && !gitVersion.startsWith(gitBranch)) {
        gitVersion.append(QStringLiteral(" (") + gitBranch + QChar(')'));
    }

    return gitVersion;
}

// static
QString VersionStore::qtVersion() {
    return qVersion();
}

// static
QString VersionStore::buildFlags() {
    return kBuildFlags;
}

QStringList VersionStore::dependencyVersions() {

    // WARNING: may be inaccurate since some come from compile-time header
    // definitions instead of the actual dynamically loaded library).
    return {// Should be accurate.
            QStringLiteral("Qt: %1").arg(qVersion()),
#ifdef __BROADCAST__
            // Should be accurate.
            QStringLiteral("libshout: %1")
                    .arg(shout_version(nullptr, nullptr, nullptr)),
#endif
            QStringLiteral("PortAudio: %1 %2")
                    .arg(Pa_GetVersion())
                    .arg(Pa_GetVersionInfo()->versionText),
#ifdef __RUBBERBAND__
            // The version of the RubberBand headers Mixxx was compiled with.
            QStringLiteral("RubberBand: " RUBBERBAND_VERSION),
#endif
            // The version of the SoundTouch headers Mixxx was compiled with.
            QStringLiteral("SoundTouch: " SOUNDTOUCH_VERSION),
            // The version of the TagLib headers Mixxx was compiled with.
            QStringLiteral("TagLib: " STR(TAGLIB_MAJOR_VERSION) "." STR(
                    TAGLIB_MINOR_VERSION) "." STR(TAGLIB_PATCH_VERSION)),
            // The version of the ChromaPrint headers Mixxx was compiled with.
            QStringLiteral("ChromaPrint: " STR(CHROMAPRINT_VERSION_MAJOR) "." STR(
                    CHROMAPRINT_VERSION_MINOR) "." STR(CHROMAPRINT_VERSION_PATCH)),
            QStringLiteral("libebur128: %1").arg(ebur128Version()),
            // Should be accurate.
            QStringLiteral("Vorbis: %1").arg(vorbis_version_string()),
            // Should be accurate.
            QStringLiteral("libsndfile: %1").arg(sf_version_string()),
            // The version of the FLAC headers Mixxx was compiled with.
            QStringLiteral("FLAC: %1").arg(FLAC__VERSION_STRING),
            QStringLiteral("libmp3lame: %1").arg(get_lame_version())};
}

void VersionStore::logBuildDetails() {
    QString version = VersionStore::version();
    QString buildFlags = VersionStore::buildFlags();

    QStringList buildInfo = {
            QStringLiteral("git %1").arg(VersionStore::gitVersion()),
#ifndef DISABLE_BUILDTIME // buildtime=1, on by default
            QStringLiteral("built on: " __DATE__ " @ " __TIME__),
#endif
    };
    if (!buildFlags.isEmpty()) {
        buildInfo.append(QStringLiteral("flags: %1").arg(buildFlags.trimmed()));
    }
    QString buildInfoFormatted = QStringLiteral("(%1)").arg(buildInfo.join("; "));

    // This is the first line in mixxx.log
    qDebug().noquote() << applicationName() << version << buildInfoFormatted << "is starting...";

    const QStringList depVersions = dependencyVersions();
    qDebug() << "Compile time library versions:";
    for (const QString& depVersion : depVersions) {
        qDebug() << depVersion;
    }

    qDebug() << "QStandardPaths::writableLocation(HomeLocation):"
             << QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    qDebug() << "QStandardPaths::writableLocation(AppDataLocation):"
             << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    qDebug() << "QCoreApplication::applicationDirPath()"
             << QCoreApplication::applicationDirPath();
}
