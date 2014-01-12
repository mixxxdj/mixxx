
#include "library/searchqueryparser.h"
#include "library/queryutil.h"
#include "track/keyutils.h"

SearchQueryParser::SearchQueryParser(QSqlDatabase& database)
        : m_database(database) {
    m_textFilters << "artist"
                  << "album_artist"
                  << "album"
                  << "title"
                  << "genre"
                  << "composer"
                  << "grouping"
                  << "comment"
                  << "location";
    m_numericFilters << "year"
                     << "track"
                     << "bpm"
                     << "duration"
                     << "played"
                     << "rating"
                     << "bitrate";
    m_specialFilters << "key";

    m_fieldToSqlColumns["artist"] << "artist" << "album_artist";
    m_fieldToSqlColumns["album_artist"] << "album_artist";
    m_fieldToSqlColumns["album"] << "album";
    m_fieldToSqlColumns["title"] << "title";
    m_fieldToSqlColumns["genre"] << "genre";
    m_fieldToSqlColumns["composer"] << "composer";
    m_fieldToSqlColumns["grouping"] << "grouping";
    m_fieldToSqlColumns["comment"] << "comment";
    m_fieldToSqlColumns["year"] << "year";
    m_fieldToSqlColumns["track"] << "tracknumber";
    m_fieldToSqlColumns["bpm"] << "bpm";
    m_fieldToSqlColumns["bitrate"] << "bitrate";
    m_fieldToSqlColumns["duration"] << "duration";
    m_fieldToSqlColumns["key"] << "key";
    m_fieldToSqlColumns["key_id"] << "key_id";
    m_fieldToSqlColumns["played"] << "timesplayed";
    m_fieldToSqlColumns["rating"] << "rating";
    m_fieldToSqlColumns["location"] << "location";

    m_allFilters.append(m_textFilters);
    m_allFilters.append(m_numericFilters);
    m_allFilters.append(m_specialFilters);

    m_operatorMatcher = QRegExp("^(>|>=|=|<|<=)(.*)$");
    m_fuzzyMatcher = QRegExp(QString("^~(%1)$").arg(m_allFilters.join("|")));
    m_textFilterMatcher = QRegExp(QString("^(%1):(.*)$").arg(m_textFilters.join("|")));
    m_numericFilterMatcher = QRegExp(QString("^(%1):(.*)$").arg(m_numericFilters.join("|")));
    m_specialFilterMatcher = QRegExp(QString("^~?(%1):(.*)$").arg(m_specialFilters.join("|")));
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
    Q_UNUSED(output);
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
                                        QStringList* output) const {
    QStringList sqlColumns = m_fieldToSqlColumns.value(field);
    if (sqlColumns.isEmpty()) {
        return false;
    }

    FieldEscaper escaper(m_database);
    QString escapedArgument = escaper.escapeString("%" + argument + "%");
    QStringList searchClauses;
    foreach (const QString sqlColumn, sqlColumns) {
        searchClauses << QString("(%1 LIKE %2)").arg(sqlColumn,
                                                     escapedArgument);
    }
    *output << (searchClauses.length() > 1 ?
                QString("(%1)").arg(searchClauses.join(" OR ")) :
                searchClauses[0]);
    return true;
}

bool SearchQueryParser::parseNumericFilter(QString field, QString argument,
                                           QStringList* output) const {
    QStringList sqlColumns = m_fieldToSqlColumns.value(field);
    if (sqlColumns.isEmpty()) {
        return false;
    }

    QString op = "=";
    if (m_operatorMatcher.indexIn(argument) != -1) {
        op = m_operatorMatcher.cap(1);
        argument = m_operatorMatcher.cap(2);
    }

    bool parsed = false;
    // Try to convert to see if it parses.
    argument.toDouble(&parsed);
    if (parsed) {
        QStringList searchClauses;
        foreach (const QString sqlColumn, sqlColumns) {
            searchClauses << QString("(%1 %2 %3)").arg(sqlColumn, op, argument);
        }
        *output << (searchClauses.length() > 1 ?
                QString("(%1)").arg(searchClauses.join(" OR ")) :
                searchClauses[0]);
        return true;
    }

    QStringList rangeArgs = argument.split("-");
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

        QStringList searchClauses;
        foreach (const QString sqlColumn, sqlColumns) {
            searchClauses << QString("(%1 >= %2 AND %1 <= %3)")
                    .arg(sqlColumn, rangeArgs[0], rangeArgs[1]);
        }

        *output << (searchClauses.length() > 1 ?
                QString("(%1)").arg(searchClauses.join(" OR ")) :
                searchClauses[0]);
        return true;
    }
    return false;
}

bool SearchQueryParser::parseSpecialFilter(QString field, QString argument,
                                           bool fuzzy,
                                           QStringList* output) const {
    if (field == "key") {
        mixxx::track::io::key::ChromaticKey key =
                KeyUtils::guessKeyFromText(argument);
        if (key == mixxx::track::io::key::INVALID) {
            return parseTextFilter(field, argument, output);
        }

        QList<mixxx::track::io::key::ChromaticKey> matchKeys;
        if (fuzzy) {
            matchKeys = KeyUtils::getCompatibleKeys(key);
        } else {
            matchKeys.push_back(key);
        }

        QStringList searchClauses;
        foreach (mixxx::track::io::key::ChromaticKey match, matchKeys) {
            searchClauses << QString("(key_id IS %1)").arg(QString::number(match));
        }

        *output << (searchClauses.length() > 1 ?
                    QString("(%1)").arg(searchClauses.join(" OR ")) :
                    searchClauses[0]);
        return true;
    }
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
            QString argument = getTextArgument(
                m_textFilterMatcher.cap(2), &tokens).trimmed();
            if (!argument.isEmpty() && parseTextFilter(
                    m_textFilterMatcher.cap(1),
                    argument, output)) {
                consumed = true;
            }
        }

        if (!consumed && m_numericFilterMatcher.indexIn(token) != -1) {
            QString argument = getTextArgument(
                m_numericFilterMatcher.cap(2), &tokens).trimmed();
            if (!argument.isEmpty() && parseNumericFilter(
                    m_numericFilterMatcher.cap(1), argument, output)) {
                consumed = true;
            }
        }

        if (!consumed && m_specialFilterMatcher.indexIn(token) != -1) {
            QString argument = getTextArgument(
                m_specialFilterMatcher.cap(2), &tokens).trimmed();
            if (!argument.isEmpty() && parseSpecialFilter(
                    m_specialFilterMatcher.cap(1), argument,
                    token.startsWith("~"), output)) {
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

    if (!extraFilter.isNull() && !extraFilter.isEmpty()) {
        queryFragments << QString("(%1)").arg(extraFilter);
    }

    if (!query.isNull() && !query.isEmpty()) {
        QStringList tokens = query.split(" ");
        parseTokens(tokens, searchColumns, &queryFragments);
    }

    QString result = "";
    if (queryFragments.size() > 0) {
        result = "WHERE " + queryFragments.join(" AND ");
    }
    //qDebug() << "Query: \"" << query << "\" parsed to:" << result;
    return result;
}
