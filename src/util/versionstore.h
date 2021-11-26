#pragma once

#include <QDateTime>
#include <QString>
#include <QVersionNumber>

class VersionStore {
  public:
    /// Returns the current Mixxx version string (e.g. 1.12.0-alpha)
    static QString version();

    /// Returns the current Mixxx version number (e.g. 1.12.0)
    static QVersionNumber versionNumber();

    /// Returns the current Mixxx version suffix (e.g. "beta")
    static QString versionSuffix();

    /// Returns the application name. (e.g. "Mixxx")
    static QString applicationName();

    /// Returns the last change date
    static QDateTime date();

    /// Returns the platform (e.g. "Windows x86_64")
    static QString platform();

    /// Returns the git branch (e.g. features_key) or the null
    /// string if the branch is unknown.
    static QString gitBranch();

    /// Returns the output of "git describe"
    static QString gitDescribe();

    /// Returns the output of "git describe" and the branch name (if available)
    static QString gitVersion();

    /// Returns the build flags used to build Mixxx (e.g. "hid=1 modplug=0") or
    /// the null string if the flags are unknown.
    static QString buildFlags();

    /// Returns a list of the version of each dependency:
    static QStringList dependencyVersions();

    /// Prints out diagnostic information about this build.
    static void logBuildDetails();
};
