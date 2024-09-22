#pragma once

#include <compare>

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

    // Do not change the order because the synthesized comparison operators
    // depend on it!
    unsigned int majorVersion;
    unsigned int minorVersion;
    unsigned int patchVersion;

  private:
    // you should not be able to create an invalid version easily from the outside
    constexpr SemanticVersion()
            : SemanticVersion(0, 0, 0){};
};

constexpr std::strong_ordering operator<=>(
        const SemanticVersion&, const SemanticVersion&) noexcept = default;

} // namespace mixxx
