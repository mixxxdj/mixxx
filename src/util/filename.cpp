#include "util/filename.h"

#include <QRegularExpression>
#include <QStringList>

namespace {
const QStringList forbiddenCharacters = {
        QStringLiteral("<"),
        QStringLiteral(">"),
        QStringLiteral("."),
        QStringLiteral(":"),
        QStringLiteral("\""),
        QStringLiteral("\'"),
        QStringLiteral("|"),
        QStringLiteral("?"),
        QStringLiteral("*"),
        QStringLiteral("\\"),
        QStringLiteral("/"),
};
const QRegularExpression forbiddenWindowsFileNames(
        QStringLiteral("^(?i)(CON|PRN|AUX|NUL|COM\\d|LPT\\d)$"));
const QString replacement = QStringLiteral("-");
} // anonymous namespace

namespace mixxx {

namespace filename {

QString sanitize(const QString& unsanitizedName) {
    QString sanitizedName = unsanitizedName;
    for (const auto& character : forbiddenCharacters) {
        sanitizedName.replace(character, replacement);
    }
    if (sanitizedName.contains(forbiddenWindowsFileNames)) {
        sanitizedName.append(replacement);
    }
    return sanitizedName;
}

} // namespace filename

} // namespace mixxx
