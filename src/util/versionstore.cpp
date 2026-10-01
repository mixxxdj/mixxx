#include "util/versionstore.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QSysInfo>
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

/// Formats one "key  value" line of the diagnostic info dump. The key is
/// left-justified so that all values start at the same column.
QString formatDiagnosticLine(
        const QString& key, const QString& value, qsizetype keyWidth) {
    return key.leftJustified(keyWidth) + QChar(' ') + value;
}

/// Matches the values of the -ffile-prefix-map and -fmacro-prefix-map
/// compiler flags, which contain private file system paths of the build
/// machine (e.g. the location of the source tree). The flags are separated
/// by whitespace or semicolons in the MIXXX_BUILD_FLAGS string.
const QRegularExpression kPrefixMapFlagRegex(
        QStringLiteral("(-ffile-prefix-map=|-fmacro-prefix-map=)[^;\\s]+"));

/// Returns the value of the given key from an os-release file
/// (https://www.freedesktop.org/software/systemd/man/latest/os-release.html)
/// or the null string if the file cannot be read or does not contain the
/// key. The values of the keys may be wrapped in double or single quotes.
QString readOsReleaseValue(const QString& osReleasePath, const QString& key) {
    QFile file(osReleasePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    const QByteArray keyWithSeparator = key.toUtf8() + '=';
    while (!file.atEnd()) {
        const QByteArray line = file.readLine().trimmed();
        if (!line.startsWith(keyWithSeparator)) {
            continue;
        }
        QByteArray value = line.mid(keyWithSeparator.size());
        if (value.size() >= 2 &&
                (value.startsWith('"') || value.startsWith('\''))) {
            value.chop(1);
            value.remove(0, 1);
        }
        return QString::fromUtf8(value);
    }
    return QString();
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
QString VersionStore::distribution() {
    return distribution(QStringLiteral("/etc/os-release"));
}

// static
QString VersionStore::distribution(const QString& osReleasePath) {
#ifdef __LINUX__
    // Prefer the PRETTY_NAME field of /etc/os-release, which contains a
    // human-readable distribution name, e.g. "Ubuntu Studio 26.04 LTS".
    // Note that inside a Flatpak sandbox this may report the runtime
    // instead of the host distribution, which is an acceptable fallback.
    const QString prettyName =
            readOsReleaseValue(osReleasePath, QStringLiteral("PRETTY_NAME"));
    if (!prettyName.isEmpty()) {
        return prettyName;
    }
    // Fall back to QSysInfo if the os-release file is unreadable or does
    // not provide a PRETTY_NAME field.
    const QString productType = QSysInfo::productType();
    const QString productVersion = QSysInfo::productVersion();
    if (productType.isEmpty() || productType == QLatin1String("unknown") ||
            productVersion.isEmpty() ||
            productVersion == QLatin1String("unknown")) {
        return QString();
    }
    return productType + QChar(' ') + productVersion;
#else
    Q_UNUSED(osReleasePath);
    return QString();
#endif
}

// static
QString VersionStore::kernel() {
    const QString kernelType = QSysInfo::kernelType();
    const QString kernelVersion = QSysInfo::kernelVersion();
    if (kernelType.isEmpty() || kernelVersion.isEmpty() ||
            kernelType == QLatin1String("unknown") ||
            kernelVersion == QLatin1String("unknown")) {
        return QString();
    }
    return kernelType + QChar(' ') + kernelVersion;
}

// static
QString VersionStore::cpuArchitecture() {
    const QString architecture = QSysInfo::currentCpuArchitecture();
    if (architecture.isEmpty() || architecture == QLatin1String("unknown")) {
        return QString();
    }
    return architecture;
}

// static
QString VersionStore::sessionType() {
#ifdef __LINUX__
    // XDG_SESSION_TYPE is the standard way to determine the type of the
    // graphical session on Linux ("x11", "wayland", "tty", ...).
    const QString sessionType = qEnvironmentVariable("XDG_SESSION_TYPE");
    if (sessionType.compare(QLatin1String("wayland"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("Wayland");
    }
    if (sessionType.compare(QLatin1String("x11"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("X11");
    }
    // Session types other than Wayland and X11 are omitted instead of being
    // reported with a placeholder value.
#endif
    // The session type is an XDG-specific concept. There is no meaningful
    // equivalent to report on other platforms.
    return QString();
}

// static
QString VersionStore::installationMethod() {
    return installationMethod(QCoreApplication::applicationDirPath());
}

// static
QString VersionStore::installationMethod(const QString& applicationDirPath) {
#ifdef __LINUX__
    const QString canonicalApplicationDir =
            QFileInfo(applicationDirPath).canonicalFilePath();

    // Flatpak: the Flatpak runtime sets FLATPAK_ID inside the sandbox.
    // https://docs.flatpak.org/en/latest/variables.html
    if (!qEnvironmentVariableIsEmpty("FLATPAK_ID")) {
        return QStringLiteral("Flatpak");
    }

    // Snap: the Snap runtime provides a set of SNAP_* variables, e.g. SNAP_NAME.
    // https://snapcraft.io/docs/environment-variables
    if (!qEnvironmentVariableIsEmpty("SNAP_NAME")) {
        return QStringLiteral("Snap");
    }

    // AppImage: the official AppImage runtime sets APPIMAGE to the absolute
    // path of the running AppImage file.
    // https://docs.appimage.org/packaging/environment-variables.html
    if (!qEnvironmentVariableIsEmpty("APPIMAGE")) {
        return QStringLiteral("AppImage");
    }

#ifdef MIXXX_BUILD_DIR
    // Uninstalled source builds are run from inside the CMake build directory,
    // which is passed to the compiler as MIXXX_BUILD_DIR. Note that a source
    // build installed with `make install` is indistinguishable from a
    // distribution package and is reported as such.
    const QString buildDirPath = QStringLiteral(MIXXX_BUILD_DIR);
    if (canonicalApplicationDir ==
            QFileInfo(buildDirPath).canonicalFilePath()) {
        return QStringLiteral("source build");
    }
#endif

    // Native Linux packages: if the executable is located in a well-known
    // system-wide binary directory we assume it was installed by a
    // distribution package (or locally via `make install`, see above).
    // Anything else, e.g. a portable archive extracted to an arbitrary
    // location, cannot be identified reliably and is omitted.
    static const QStringList kSystemBinDirs = {
            QStringLiteral("/usr/bin"),
            QStringLiteral("/usr/local/bin"),
    };
    for (const QString& systemBinDir : kSystemBinDirs) {
        if (canonicalApplicationDir ==
                QFileInfo(systemBinDir).canonicalFilePath()) {
            return QStringLiteral("distro package");
        }
    }
#else
    Q_UNUSED(applicationDirPath);
#endif
    // On Windows and macOS the installation method (installer, portable
    // archive, DMG, App bundle, Homebrew, ...) cannot be detected reliably,
    // so the entry is omitted entirely.
    return QString();
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

// static
QString VersionStore::maskedBuildFlags() {
    return maskedBuildFlags(buildFlags());
}

// static
QString VersionStore::maskedBuildFlags(const QString& buildFlags) {
    // The values of the -ffile-prefix-map and -fmacro-prefix-map compiler
    // flags contain private file system paths of the build machine (e.g. the
    // location of the source tree), which must not be shared in bug reports.
    return QString(buildFlags).replace(kPrefixMapFlagRegex, QStringLiteral("\\1<masked>"));
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

// static
QString VersionStore::diagnosticInfo(const QString& skin) {
    // This dump is a technical text for bug reports. The keys are stable
    // English identifiers and must NOT be translated.
    QList<QPair<QString, QString>> entries;
    entries.append({QStringLiteral("Version"), version()});

    // gitVersion() also contains the branch name (if available), but falls
    // back to "unknown" if the build does not provide the git information.
    if (!gitDescribe().isEmpty()) {
        entries.append({QStringLiteral("Git version"), gitVersion()});
    }

    const QDateTime lastCommitDate = date();
    if (lastCommitDate.isValid()) {
        // This is the date of the last commit of the source tree the binary
        // was built from, not the build date. Use UTC to make the date
        // unambiguous in bug reports.
        entries.append({QStringLiteral("Last commit"),
                lastCommitDate.toUTC().toString(Qt::ISODate)});
    }

    entries.append({QStringLiteral("Qt"), qtVersion()});

#ifdef __LINUX__
    const QString distribution = VersionStore::distribution();
    if (!distribution.isEmpty()) {
        entries.append({QStringLiteral("Distribution"), distribution});
    }
#else
    const QString productName = QSysInfo::prettyProductName();
    if (!productName.isEmpty() && productName != QLatin1String("unknown")) {
        entries.append({QStringLiteral("OS"), productName});
    }
#endif

    const QString kernel = VersionStore::kernel();
    if (!kernel.isEmpty()) {
        entries.append({QStringLiteral("Kernel"), kernel});
    }

    const QString cpuArchitecture = VersionStore::cpuArchitecture();
    if (!cpuArchitecture.isEmpty()) {
        entries.append({QStringLiteral("Architecture"), cpuArchitecture});
    }

    const QString session = VersionStore::sessionType();
    if (!session.isEmpty()) {
        entries.append({QStringLiteral("Session type"), session});
    }

    const QString installMethod = VersionStore::installationMethod();
    if (!installMethod.isEmpty()) {
        entries.append({QStringLiteral("Installation method"), installMethod});
    }

    if (!skin.isEmpty()) {
        entries.append({QStringLiteral("Skin"), skin});
    }

    const QString buildFlags = VersionStore::maskedBuildFlags();
    if (!buildFlags.isEmpty()) {
        entries.append({QStringLiteral("Build flags"), buildFlags});
    }

    const QStringList dependencies = dependencyVersions();
    if (!dependencies.isEmpty()) {
        entries.append({QStringLiteral("Dependencies"),
                dependencies.join(QStringLiteral(", "))});
    }

    qsizetype keyWidth = 0;
    for (const auto& entry : entries) {
        keyWidth = qMax(keyWidth, entry.first.length());
    }

    QStringList lines;
    lines.reserve(entries.size());
    for (const auto& entry : entries) {
        lines.append(formatDiagnosticLine(entry.first, entry.second, keyWidth));
    }
    return lines.join(QChar('\n'));
}
