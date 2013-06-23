#ifndef VERSION_H
#define VERSION_H

#include <QString>

class Version {
  public:
    // Returns the current Mixxx version (e.g. 1.12.0-alpha)
    static QString version();

    // Returns the development branch (e.g. features_key) or the null
    // string if the branch is unknown.
    static QString developmentBranch();

    // Returns the development revision (e.g. git3096) or the null string if the
    // revision is unknown.
    static QString developmentRevision();

    // Returns the build flags used to build Mixxx (e.g. "hid=1 modplug=0") or
    // the null string if the flags are unknown.
    static QString buildFlags();
};

#endif /* VERSION_H */
