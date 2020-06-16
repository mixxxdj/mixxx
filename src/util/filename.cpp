#include "util/filename.h"

#include <QStringList>

namespace mixxx {

namespace filename {

QString sanitize(const QString& unsanitizedName) {
    QString sanitizedName = unsanitizedName;
    sanitizedName.replace("/", "-");
    QStringList forbiddenCharacters;
    forbiddenCharacters << "<"
                        << ">"
                        << ":"
                        << "\""
                        << "\'"
                        << "|"
                        << "?"
                        << "*"
                        << "\\";
    for (const auto& character : forbiddenCharacters) {
        sanitizedName.remove(character);
    }
    return sanitizedName;
}

} // namespace filename

} // namespace mixxx
