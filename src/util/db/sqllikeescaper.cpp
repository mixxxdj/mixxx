#include "util/db/sqllikeescaper.h"


QString SqlLikeEscaper::apply(const QString& escapeString, QChar escapeCharacter) {
    QString escapeCharacterStr(escapeCharacter);
    QString result = escapeString;
    // Replace instances of escapeCharacter with two escapeCharacters.
    result = result.replace(
        escapeCharacter, escapeCharacterStr + escapeCharacterStr);
    // Replace instances of % or _ with $escapeCharacter%.
    if (escapeCharacter != '%') {
        result = result.replace("%", escapeCharacterStr + "%");
    }
    if (escapeCharacter != '_') {
        result = result.replace("_", escapeCharacterStr + "_");
    }
    return result;
}
