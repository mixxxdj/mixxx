
#include "library/searchqueryparser.h"
#include "library/queryutil.h"

SearchQueryParser::SearchQueryParser(QSqlDatabase& database)
        : m_database(database) {
    m_textFilters << "artist"
                  << "album"
                  << "title"
                  << "genre"
                  << "composer"
                  << "comment"
                  << "key";
    m_numericFilters << "year"
                     << "track"
                     << "bpm"
                     << "duration"
                     << "played"
                     << "rating"
                     << "bitrate";
    // TODO(XXX): For now key search is text-only. In the future we want to
    // support multiple notations of key searching.
    //m_specialFilters << "key";

    m_fieldToSqlColumn.insert("artist", "artist");
    m_fieldToSqlColumn.insert("album", "album");
    m_fieldToSqlColumn.insert("title", "title");
    m_fieldToSqlColumn.insert("genre", "genre");
    m_fieldToSqlColumn.insert("composer", "composer");
    m_fieldToSqlColumn.insert("comment", "comment");
    m_fieldToSqlColumn.insert("year", "year");
    m_fieldToSqlColumn.insert("track", "tracknumber");
    m_fieldToSqlColumn.insert("bpm", "bpm");
    m_fieldToSqlColumn.insert("bitrate", "bitrate");
    m_fieldToSqlColumn.insert("duration", "duration");
    m_fieldToSqlColumn.insert("key", "key");
    m_fieldToSqlColumn.insert("played", "timesplayed");
    m_fieldToSqlColumn.insert("rating", "rating");

    m_allFilters.append(m_textFilters);
    m_allFilters.append(m_numericFilters);
    m_allFilters.append(m_specialFilters);

    m_operatorMatcher = QRegExp("^(>|>=|=|<|<=)(.*)$");
    m_fuzzyMatcher = QRegExp(QString("^~(%1)$").arg(m_allFilters.join("|")));
    m_textFilterMatcher = QRegExp(QString("^(%1):(.*)$").arg(m_textFilters.join("|")));
    m_numericFilterMatcher = QRegExp(QString("^(%1):(.*)$").arg(m_numericFilters.join("|")));
    m_specialFilterMatcher = QRegExp(QString("^(%1):(.*)$").arg(m_specialFilters.join("|")));
}

SearchQueryParser::~SearchQueryParser() {
}

bool SearchQueryParser::searchFieldsForPhrase(const QString& phrase,
                                              const QStringList& fields,
                                              QStringList* output) const {
    FieldEscaper escaper(m_database);
    QString escapedPhrase = escaper.escapeString("%" + phrase + "%");

    QStringList fieldFragments;
    foreach (QString field, fields) {
        fieldFragments << QString("(%1 LIKE %2)").arg(field, escapedPhrase);
    }
    *output << QString("(%1)").arg(fieldFragments.join(" OR "));
    return true;
}

bool SearchQueryParser::parseFuzzyMatch(QString field, QStringList* output) const {
    if (field == "bpm") {
        // Look up the current track's bpms and add something like
        // "bpm > X AND bpm" < Y to 'output'

        // Make sure to return true if you processed successfully
        //return true;
    } else if (field == "key") {
        // Look up current track's key and add something like
        // "key" in (.. list of compatible keys.. )

        // Make sure to return true if you processed successfully
        //return true;
    }

    return false;
}

QString SearchQueryParser::getTextArgument(QString argument,
                                           QStringList* tokens) const {
    // If the argument is empty, assume the user placed a space after an
    // advanced search command. Consume another token and treat that as the
    // argument. TODO(XXX) support quoted search phrases as arguments
    argument = argument.trimmed();
    if (argument.length() == 0) {
        if (tokens->length() > 0) {
            argument = tokens->takeFirst();
        }
    }

    // Deal with quoted arguments. If this token started with a quote, then
    // search for the closing quote.
    if (argument.startsWith("\"")) {
        argument = argument.mid(1);

        int quote_index = argument.indexOf("\"");
        while (quote_index == -1 && tokens->length() > 0) {
            argument += " " + tokens->takeFirst();
            quote_index = argument.indexOf("\"");
        }

        if (quote_index == -1) {
            // No ending quote found. Since we think they are going to close the
            // quote eventually, treat the entire token list as the argument for
            // now.
            return argument;
        }

        // Stuff the rest of the argument after the quote back into tokens.
        QString remaining = argument.mid(quote_index+1).trimmed();
        if (remaining.size() != 0) {
            tokens->push_front(remaining);
        }

        // Slice off the quote and everything after.
        argument = argument.left(quote_index);
    }

    return argument;
}

bool SearchQueryParser::parseTextFilter(QString field, QString argument,
                                        QStringList* tokens,
                                        QStringList* output) const {
    QString sqlField = m_fieldToSqlColumn.value(field, "");
    if (sqlField.length() == 0) {
        return false;
    }

    QString filter = getTextArgument(argument, tokens).trimmed();
    if (filter.length() == 0) {
        qDebug() << "Text filter for" << field << "was empty.";
        return false;
    }

    FieldEscaper escaper(m_database);
    QString escapedFilter = escaper.escapeString("%" + filter + "%");
    *output << QString("(%1 LIKE %2)").arg(sqlField, escapedFilter);
    return true;
}

bool SearchQueryParser::parseNumericFilter(QString field, QString argument,
                                           QStringList* tokens,
                                           QStringList* output) const {
    QString sqlField = m_fieldToSqlColumn.value(field, "");
    if (sqlField.length() == 0) {
        return false;
    }

    QString filter = getTextArgument(argument, tokens).trimmed();
    if (filter.length() == 0) {
        qDebug() << "Text filter for" << field << "was empty.";
        return false;
    }


    QString op = "=";
    if (m_operatorMatcher.indexIn(filter) != -1) {
        op = m_operatorMatcher.cap(1);
        filter = m_operatorMatcher.cap(2);
    }

    bool parsed = false;
    // Try to convert to see if it parses.
    filter.toDouble(&parsed);
    if (parsed) {
        *output << QString("(%1 %2 %3)").arg(sqlField, op, filter);
        return true;
    }

    QStringList rangeArgs = filter.split("-");
    if (rangeArgs.length() == 2) {
        bool ok = false;
        double arg1 = rangeArgs[0].toDouble(&ok);
        if (!ok) {
            return false;
        }
        double arg2 = rangeArgs[1].toDouble(&ok);
        if (!ok) {
            return false;
        }

        // Nonsense
        if (arg1 > arg2) {
            return false;
        }

        *output << QString("(%1 >= %2 AND %1 <= %3)").arg(
            sqlField, rangeArgs[0], rangeArgs[1]);
        return true;
    }
    return false;
}

bool SearchQueryParser::parseSpecialFilter(QString field, QString argument,
                                           QStringList* tokens,
                                           QStringList* output) const {
    // Currently unimplemented. This is called when we want to filter by
    // key. The handling here will depend on how we represent the keys in the
    // database but it should ideally automatically deal with all the different
    // representations there are for keys.
    return false;
}

void SearchQueryParser::parseTokens(QStringList tokens,
                                    QStringList searchColumns,
                                    QStringList* output) const {
    while (tokens.size() > 0) {
        QString token = tokens.takeFirst().trimmed();
        if (token.length() == 0) {
            continue;
        }

        bool consumed = false;

        if (!consumed && m_fuzzyMatcher.indexIn(token) != -1) {
            if (parseFuzzyMatch(m_fuzzyMatcher.cap(1), output)) {
                consumed = true;
            }
        }

        if (!consumed && m_textFilterMatcher.indexIn(token) != -1) {
            if (parseTextFilter(m_textFilterMatcher.cap(1),
                                m_textFilterMatcher.cap(2),
                                &tokens, output)) {
                consumed = true;
            }
        }

        if (!consumed && m_numericFilterMatcher.indexIn(token) != -1) {
            if (parseNumericFilter(m_numericFilterMatcher.cap(1),
                                   m_numericFilterMatcher.cap(2),
                                   &tokens, output)) {
                consumed = true;
            }
        }

        if (!consumed && m_specialFilterMatcher.indexIn(token) != -1) {
            if (parseSpecialFilter(m_specialFilterMatcher.cap(1),
                                   m_specialFilterMatcher.cap(2),
                                   &tokens, output)) {
                consumed = true;
            }
        }

        // If no advanced search feature matched, treat it as a search term.
        if (!consumed) {
            if (searchFieldsForPhrase(token, searchColumns, output)) {
                consumed = true;
            }
        }
    }
}

QString SearchQueryParser::parseQuery(const QString& query,
                                      const QStringList& searchColumns,
                                      const QString& extraFilter) const {
    FieldEscaper escaper(m_database);
    QStringList queryFragments;

    if (!extraFilter.isNull() && extraFilter != "") {
        queryFragments << QString("(%1)").arg(extraFilter);
    }

    if (!query.isNull() && query != "") {
        QStringList tokens = query.split(" ");
        parseTokens(tokens, searchColumns, &queryFragments);
    }

    QString result = "";
    if (queryFragments.size() > 0) {
        result = "WHERE " + queryFragments.join(" AND ");
    }
    qDebug() << "Query: \"" << query << "\" parsed to:" << result;
    return result;
}
