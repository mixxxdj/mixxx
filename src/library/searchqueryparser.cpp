#include "library/searchqueryparser.h"

#include "track/keyutils.h"

const char* kNegatePrefix = "-";
const char* kFuzzyPrefix = "~";

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
                     << "played"
                     << "rating"
                     << "bitrate";
    m_specialFilters << "key"
                     << "duration";

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

    m_fuzzyMatcher = QRegExp(QString("^~(%1)$").arg(m_allFilters.join("|")));
    m_textFilterMatcher = QRegExp(QString("^-?(%1):(.*)$").arg(m_textFilters.join("|")));
    m_numericFilterMatcher = QRegExp(QString("^-?(%1):(.*)$").arg(m_numericFilters.join("|")));
    m_specialFilterMatcher = QRegExp(QString("^[~-]?(%1):(.*)$").arg(m_specialFilters.join("|")));
}

SearchQueryParser::~SearchQueryParser() {
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

void SearchQueryParser::parseTokens(QStringList tokens,
                                    QStringList searchColumns,
                                    AndNode* pQuery) const {
    while (tokens.size() > 0) {
        QString token = tokens.takeFirst().trimmed();
        if (token.length() == 0) {
            continue;
        }

        if (m_fuzzyMatcher.indexIn(token) != -1) {
            // TODO(XXX): implement this feature.
        } else if (m_textFilterMatcher.indexIn(token) != -1) {
            bool negate = token.startsWith(kNegatePrefix);
            QString field = m_textFilterMatcher.cap(1);
            QString argument = getTextArgument(
                m_textFilterMatcher.cap(2), &tokens).trimmed();

            if (!argument.isEmpty()) {
                QueryNode* pNode = new TextFilterNode(
                    m_database, m_fieldToSqlColumns[field], argument);
                if (negate) {
                    pNode = new NotNode(pNode);
                }
                pQuery->addNode(pNode);
            }
        } else if (m_numericFilterMatcher.indexIn(token) != -1) {
            bool negate = token.startsWith(kNegatePrefix);
            QString field = m_numericFilterMatcher.cap(1);
            QString argument = getTextArgument(
                m_numericFilterMatcher.cap(2), &tokens).trimmed();

            if (!argument.isEmpty()) {
                QueryNode* pNode = new NumericFilterNode(
                    m_fieldToSqlColumns[field], argument);
                if (negate) {
                    pNode = new NotNode(pNode);
                }
                pQuery->addNode(pNode);
            }
        } else if (m_specialFilterMatcher.indexIn(token) != -1) {
            bool negate = token.startsWith(kNegatePrefix);
            bool fuzzy = token.startsWith(kFuzzyPrefix);
            QString field = m_specialFilterMatcher.cap(1);
            QString argument = getTextArgument(
                m_specialFilterMatcher.cap(2), &tokens).trimmed();
            QueryNode* pNode = NULL;
            if (!argument.isEmpty()) {
                if (field == "key") {
                    mixxx::track::io::key::ChromaticKey key =
                            KeyUtils::guessKeyFromText(argument);
                    if (key == mixxx::track::io::key::INVALID) {
                        pNode = new TextFilterNode(
                            m_database, m_fieldToSqlColumns[field], argument);
                    } else {
                        pNode = new KeyFilterNode(key, fuzzy);
                    }
                } else if (field == "duration") {
                    pNode = new DurationFilterNode(m_fieldToSqlColumns[field],
                                                   argument);
                }
            }
            if (pNode != NULL) {
                if (negate) {
                    pNode = new NotNode(pNode);
                }
                pQuery->addNode(pNode);
            }
        } else {
            // If no advanced search feature matched, treat it as a search term.
            bool negate = token.startsWith(kNegatePrefix);
            if (negate) {
                token = token.mid(1);
            }

            // Don't trigger on a lone minus sign.
            if (!token.isEmpty()) {
                QueryNode* pNode = new TextFilterNode(
                    m_database, searchColumns, token);
                if (negate) {
                    pNode = new NotNode(pNode);
                }
                pQuery->addNode(pNode);
            }
        }
    }
}

QueryNode* SearchQueryParser::parseQuery(const QString& query,
                                         const QStringList& searchColumns,
                                         const QString& extraFilter) const {
    AndNode* pQuery = new AndNode();

    if (!extraFilter.isEmpty()) {
        pQuery->addNode(new SqlNode(extraFilter));
    }

    if (!query.isEmpty()) {
        QStringList tokens = query.split(" ");
        parseTokens(tokens, searchColumns, pQuery);
    }

    return pQuery;
}
