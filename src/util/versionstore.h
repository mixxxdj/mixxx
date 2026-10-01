#pragma once

#include <QDateTime>
#include <QString>
#include <QVersionNumber>

namespace VersionStore {
/// Returns the current Mixxx version string (e.g. 1.12.0-alpha)
QString version();

/// Returns the current Mixxx version number (e.g. 1.12.0)
QVersionNumber versionNumber();

/// Returns the current Mixxx version suffix (e.g. "beta")
QString versionSuffix();

/// Returns the application name. (e.g. "Mixxx")
QString applicationName();

/// Returns the last change date
QDateTime date();

/// Returns the platform (e.g. "Windows x86_64")
QString platform();

/// Returns the name of the Linux distribution, preferring the human
/// readable PRETTY_NAME field of /etc/os-release and falling back to
/// QSysInfo. Returns the null string on other platforms or if the
/// distribution cannot be determined.
QString distribution();

/// Same as distribution(), but takes the path of the os-release file as
/// a parameter for testing purposes.
QString distribution(const QString& osReleasePath);

/// Returns the type and version of the operating system kernel
/// (e.g. "linux 6.14.0-8-generic") or the null string if the kernel
/// information is not available.
QString kernel();

/// Returns the CPU architecture the running binary was built for
/// (e.g. "x86_64") or the null string if it cannot be determined.
QString cpuArchitecture();

/// Returns the type of the graphical session (e.g. "Wayland" or "X11") or
/// the null string if the session type is unknown. This is only detected
/// on Linux via XDG_SESSION_TYPE; on all other platforms the null string
/// is returned.
QString sessionType();

/// Returns how this copy of Mixxx was installed (e.g. "Flatpak", "Snap",
/// "AppImage", "distro package" or "source build") or the null string if
/// the installation method cannot be determined reliably.
QString installationMethod();

/// Same as installationMethod(), but takes the directory of the running
/// executable as a parameter for testing purposes.
QString installationMethod(const QString& applicationDirPath);

/// Returns the git branch (e.g. features_key) or the null
/// string if the branch is unknown.
QString gitBranch();

/// Returns the output of "git describe"
QString gitDescribe();

/// Returns the output of "git describe" and the branch name (if available)
QString gitVersion();

/// Returns the version of Qt used to build Mixxx.
QString qtVersion();

/// Returns the build flags used to build Mixxx (e.g. "hid=1 modplug=0") or
/// the null string if the flags are unknown.
QString buildFlags();

/// Returns the build flags with the values of the -ffile-prefix-map and
/// -fmacro-prefix-map compiler flags masked out, as these contain
/// private file system paths of the build machine.
QString maskedBuildFlags();

/// Same as maskedBuildFlags(), but takes the raw build flags as a
/// parameter for testing purposes.
QString maskedBuildFlags(const QString& buildFlags);

/// Returns a list of the version of each dependency:
QStringList dependencyVersions();

/// Prints out diagnostic information about this build.
void logBuildDetails();

/// Returns a plain-text key-value dump of the version, build and system
/// information for pasting it into bug reports. The keys are stable
/// English identifiers and must not be translated. The skin parameter can
/// pass the name of the currently configured skin; if it is empty the skin
/// entry is omitted.
QString diagnosticInfo(const QString& skin = QString());
} // namespace VersionStore
