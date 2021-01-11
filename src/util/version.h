#pragma once

#include <QString>

class Version {
  public:

    // Returns the application name. (e.g. "Mixxx")
    static QString applicationName();

    // Returns the application title (e.g. "Mixxx x64" on Windows)
    static QString applicationTitle();

    /// most recent Git tag
    static QString gitTag();

    /// unique identifier of Git commit including most recent tag name,
    /// number of commits since the tag, and commit hash
    static QString gitCommitDescription();

    static QString gitCommitDate();

    // Returns the build flags used to build Mixxx (e.g. "hid=1 modplug=0") or
    // the null string if the flags are unknown.
    static QString buildFlags();

    // Returns a list of the version of each dependency:
    static QStringList dependencyVersions();

    // Prints out diagnostic information about this build.
    static void logBuildDetails();
};
