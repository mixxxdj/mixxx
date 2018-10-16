#include "util/db/sqllikewildcardescaper.h"

#include "util/db/sqllikewildcards.h"


//static
QString SqlLikeWildcardEscaper::apply(
        const QString& unescapedString,
        QChar escapeCharacter) {
    QString escapePrefix(escapeCharacter);
    QString escapedString = unescapedString;
    // Duplicate all occurrences of escapeCharacter, i.e. prefix escapeCharacter
    // with itself
    escapedString.replace(escapeCharacter, escapePrefix + escapeCharacter);
    // Prefix all occurrences of LIKE wildcards other than the escapeCharacter
    // itself with escapeCharacter
    if (escapeCharacter != kSqlLikeMatchAll) {
        escapedString.replace(kSqlLikeMatchAll, escapePrefix + kSqlLikeMatchAll);
    }
    if (escapeCharacter != kSqlLikeMatchOne) {
        escapedString.replace(kSqlLikeMatchOne, escapePrefix + kSqlLikeMatchOne);
    }
    return escapedString;
}
