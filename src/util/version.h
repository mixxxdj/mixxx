#pragma once

#include <QString>

class Version {
  public:
    // Returns the current Mixxx version (e.g. 1.12.0-alpha)
    static QString version();

    // Returns the application name. (e.g. "Mixxx")
    static QString applicationName();

    // Returns the application title (e.g. "Mixxx x64" on Windows)
    static QString applicationTitle();

    // Returns the development branch (e.g. features_key) or the null
    // string if the branch is unknown.
    static QString developmentBranch();

    // Returns the development revision (e.g. git3096) or the null string if the
    // revision is unknown.
    static QString developmentRevision();

    // Returns the build flags used to build Mixxx (e.g. "hid=1 modplug=0") or
    // the null string if the flags are unknown.
    static QString buildFlags();

    // Returns a list of the version of each dependency:
    static QStringList dependencyVersions();

    // Prints out diagnostic information about this build.
    static void logBuildDetails();
};
