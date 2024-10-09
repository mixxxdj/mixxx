#pragma once

#include <compare>
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

    friend constexpr std::strong_ordering operator<=>(
            const SemanticVersion&, const SemanticVersion&) noexcept;

    friend constexpr bool operator==(
            const SemanticVersion&, const SemanticVersion&) noexcept;

    // Do not change the order because the synthesized comparison operators
    // depend on it!
    unsigned int majorVersion;
    unsigned int minorVersion;
    unsigned int patchVersion;

  private:
    // you should not be able to create an invalid version easily from the outside
    constexpr SemanticVersion()
            : SemanticVersion(0, 0, 0){};
    // make tuple, which is used for the implementation of the comparison operators
    constexpr std::tuple<unsigned int, unsigned int, unsigned int> makeTuple() const {
        return std::tie(majorVersion, minorVersion, patchVersion);
    }
};

// TODO: replace with = default (synthesized) implementations once widely
// supported on target compilers.
constexpr std::strong_ordering operator<=>(
        const SemanticVersion& lhs, const SemanticVersion& rhs) noexcept {
    return lhs.makeTuple() <=> rhs.makeTuple();
}

constexpr bool operator==(
        const SemanticVersion& lhs, const SemanticVersion& rhs) noexcept {
    return lhs.makeTuple() == rhs.makeTuple();
}
} // namespace mixxx
