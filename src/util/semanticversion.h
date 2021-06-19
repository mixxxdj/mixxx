#pragma once

#include <QString>
#include <tuple>

namespace mixxx {

// "major" and "minor" clash with symbols in glibc, so use more verbose variable names.
class SemanticVersion {
  public:
    SemanticVersion(unsigned int majorVersion,
            unsigned int minorVersion,
            unsigned int patchVersion);
    SemanticVersion(const QString& versionString);

    bool isValid() const;

    unsigned int majorVersion, minorVersion, patchVersion;
};

inline bool operator<(const SemanticVersion& a, const SemanticVersion& b) {
    return std::tie(a.majorVersion, a.minorVersion, a.patchVersion) <
            std::tie(b.majorVersion, b.minorVersion, b.patchVersion);
}

inline bool operator>(const SemanticVersion& a, const SemanticVersion& b) {
    return b < a;
}

inline bool operator<=(const SemanticVersion& a, const SemanticVersion& b) {
    return !(a > b);
}

inline bool operator>=(const SemanticVersion& a, const SemanticVersion& b) {
    return !(a < b);
}

} // namespace mixxx
