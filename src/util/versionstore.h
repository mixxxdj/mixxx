#pragma once

#include <QDateTime>
#include <QString>
#include <QVersionNumber>

namespace VersionStore {
/// Constant to store the future unreleased Mixxx 3.0
static QString FUTURE_UNSTABLE = QStringLiteral("3.0-unstable");

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

/// Returns a list of the version of each dependency:
QStringList dependencyVersions();

/// Prints out diagnostic information about this build.
void logBuildDetails();
} // namespace VersionStore
