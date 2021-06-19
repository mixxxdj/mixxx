#include "util/semanticversion.h"

#include <QRegularExpression>

namespace {

QRegularExpression regex("(?<major>\\d+)\\.(?<minor>\\d+)\\.(?<patch>\\d+)");

} // anonymous namespace

namespace mixxx {

SemanticVersion::SemanticVersion(unsigned int majorVersion,
        unsigned int minorVersion,
        unsigned int patchVersion)
        : majorVersion(majorVersion),
          minorVersion(minorVersion),
          patchVersion(patchVersion) {
}

SemanticVersion::SemanticVersion(const QString& versionString)
        : majorVersion(0),
          minorVersion(0),
          patchVersion(0) {
    QRegularExpressionMatch match = regex.match(versionString);
    if (match.hasMatch()) {
        majorVersion = match.captured(QStringLiteral("major")).toUInt();
        minorVersion = match.captured(QStringLiteral("minor")).toUInt();
        patchVersion = match.captured(QStringLiteral("patch")).toUInt();
    }
}

bool SemanticVersion::isValid() const {
    return !(majorVersion == 0 && minorVersion == 0 && patchVersion == 0);
}

} // namespace mixxx
