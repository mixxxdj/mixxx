#pragma once

#include <QString>


// Utility class for escaping like statements.
class SqlLikeWildcardEscaper final {
  public:
    // Escapes a string for use in a LIKE operation by prefixing instances of
    // LIKE wildcard characters (% and _) with escapeCharacter. This allows the
    // caller to then attach wildcard characters to the string. This does NOT
    // escape the string in the same way that SqlStringFormatter does!
    static QString apply(const QString& unescapedString, QChar escapeCharacter);

  private:
    SqlLikeWildcardEscaper() = delete; // utility class
};
