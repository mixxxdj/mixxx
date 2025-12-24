#pragma once

#include <tuple>

class QString;

namespace mixxx {

// "major" and "minor" clash with symbols in glibc, so use more verbose variable names.
class SemanticVersion {
  public:
    constexpr SemanticVersion(unsigned int majorVersion,
            unsigned int minorVersion,
            unsigned int patchVersion)
            : majorVersion(majorVersion),
              minorVersion(minorVersion),
              patchVersion(patchVersion) {
    }

    SemanticVersion(const QString& versionString);

    constexpr bool isValid() const {
        return !(majorVersion == 0 && minorVersion == 0 && patchVersion == 0);
    }

    // make tuple, which is used for the implementation of the comparison operators
    constexpr std::tuple<unsigned int, unsigned int, unsigned int> makeTuple() const {
        return std::tie(majorVersion, minorVersion, patchVersion);
    }

    // Do not change the order because the synthesized comparison operators
    // depend on it!
    unsigned int majorVersion;
    unsigned int minorVersion;
    unsigned int patchVersion;

  private:
    // you should not be able to create an invalid version easily from the outside
    // but we need the ability internally for the QString constructor.
    constexpr SemanticVersion()
            : SemanticVersion(0, 0, 0) {};
};

constexpr bool operator==(const SemanticVersion& a, const SemanticVersion& b) {
    return a.makeTuple() == b.makeTuple();
}

// TODO: replace with std::strong_ordering operator<=> once available:

constexpr bool operator<(const SemanticVersion& a, const SemanticVersion& b) {
    return a.makeTuple() < b.makeTuple();
}

constexpr bool operator>(const SemanticVersion& a, const SemanticVersion& b) {
    return b < a;
}

constexpr bool operator<=(const SemanticVersion& a, const SemanticVersion& b) {
    return !(a > b);
}
constexpr bool operator>=(const SemanticVersion& a, const SemanticVersion& b) {
    return !(a < b);
}

} // namespace mixxx
