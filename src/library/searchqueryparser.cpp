#include "library/searchqueryparser.h"

#include <QRegularExpression>

#include "track/keyutils.h"

constexpr char kNegatePrefix[] = "-";
constexpr char kFuzzyPrefix[] = "~";
// see https://stackoverflow.com/questions/1310473/regex-matching-spaces-but-not-in-strings
const QRegularExpression kSplitIntoWordsRegexp = QRegularExpression(
        QStringLiteral(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)"));

SearchQueryParser::SearchQueryParser(TrackCollection* pTrackCollection)
    : m_pTrackCollection(pTrackCollection) {
    m_textFilters << "artist"
                  << "album_artist"
                  << "album"
                  << "title"
                  << "genre"
                  << "composer"
                  << "grouping"
                  << "comment"
                  << "location"
                  << "crate";
    m_numericFilters << "year"
                     << "track"
                     << "bpm"
                     << "played"
                     << "rating"
                     << "bitrate";
    m_specialFilters << "key"
                     << "duration"
                     << "added"
                     << "dateadded"
                     << "datetime_added"
                     << "date_added";
    m_ignoredColumns << "crate";

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
    m_fieldToSqlColumns["lastplayed"] << "last_played_at";
    m_fieldToSqlColumns["rating"] << "rating";
    m_fieldToSqlColumns["location"] << "location";
    m_fieldToSqlColumns["datetime_added"] << "datetime_added";

    m_allFilters.append(m_textFilters);
    m_allFilters.append(m_numericFilters);
    m_allFilters.append(m_specialFilters);

    m_fuzzyMatcher = QRegularExpression(QString("^~(%1)$").arg(m_allFilters.join("|")));
    m_textFilterMatcher = QRegularExpression(QString("^-?(%1):(.*)$").arg(m_textFilters.join("|")));
    m_numericFilterMatcher = QRegularExpression(
            QString("^-?(%1):(.*)$").arg(m_numericFilters.join("|")));
    m_specialFilterMatcher = QRegularExpression(
            QString("^[~-]?(%1):(.*)$").arg(m_specialFilters.join("|")));
}

SearchQueryParser::~SearchQueryParser() {
}

QString SearchQueryParser::getTextArgument(QString argument,
                                           QStringList* tokens) const {
    // If the argument is empty, assume the user placed a space after an
    // advanced search command. Consume another token and treat that as the
    // argument.
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

        if (quote_index == 0) {
            // We have found an explicit empty string ""
            // return it as "" to distinguish it from an unfinished empty string
            argument = kMissingFieldSearchTerm;
        } else {
            // Slice off the quote and everything after.
            argument = argument.left(quote_index);
        }
    }

    return argument;
}

void SearchQueryParser::parseTokens(QStringList tokens,
                                    QStringList searchColumns,
                                    AndNode* pQuery) const {
    // we need to create a filtered columns list that are handled differently
    auto queryColumns = QStringList();
    queryColumns.reserve(searchColumns.count());

    for (const auto& column: qAsConst(searchColumns)) {
        if (m_ignoredColumns.contains(column)) {
            continue;
        }
        queryColumns << column;
    }


    while (tokens.size() > 0) {
        QString token = tokens.takeFirst().trimmed();
        if (token.length() == 0) {
            continue;
        }

        bool negate = token.startsWith(kNegatePrefix);
        std::unique_ptr<QueryNode> pNode;

        const QRegularExpressionMatch fuzzyMatch = m_fuzzyMatcher.match(token);
        const QRegularExpressionMatch textFilterMatch = m_textFilterMatcher.match(token);
        const QRegularExpressionMatch numericFilterMatch = m_numericFilterMatcher.match(token);
        const QRegularExpressionMatch specialFilterMatch = m_specialFilterMatcher.match(token);
        if (fuzzyMatch.hasMatch()) {
            // TODO(XXX): implement this feature.
        } else if (textFilterMatch.hasMatch()) {
            QString field = textFilterMatch.captured(1);
            QString argument = getTextArgument(
                    textFilterMatch.captured(2), &tokens);

            if (argument == kMissingFieldSearchTerm) {
                qDebug() << "argument explicit empty";
                if (field == "crate") {
                    pNode = std::make_unique<NoCrateFilterNode>(
                          &m_pTrackCollection->crates());
                    qDebug() << pNode->toSql();
                } else {
                    pNode = std::make_unique<NullOrEmptyTextFilterNode>(
                          m_pTrackCollection->database(), m_fieldToSqlColumns[field]);
                    qDebug() << pNode->toSql();
                }
            } else if (!argument.isEmpty()) {
                if (field == "crate") {
                    pNode = std::make_unique<CrateFilterNode>(
                            &m_pTrackCollection->crates(), argument);
                } else {
                    pNode = std::make_unique<TextFilterNode>(
                            m_pTrackCollection->database(),
                            m_fieldToSqlColumns[field], argument);
                }
            }
        } else if (numericFilterMatch.hasMatch()) {
            QString field = numericFilterMatch.captured(1);
            QString argument = getTextArgument(
                    numericFilterMatch.captured(2), &tokens)
                                       .trimmed();

            if (!argument.isEmpty()) {
                if (argument == kMissingFieldSearchTerm) {
                    pNode = std::make_unique<NullNumericFilterNode>(
                         m_fieldToSqlColumns[field]);
                } else {
                    pNode = std::make_unique<NumericFilterNode>(
                         m_fieldToSqlColumns[field], argument);
                }
            }
        } else if (specialFilterMatch.hasMatch()) {
            bool fuzzy = token.startsWith(kFuzzyPrefix);
            QString field = specialFilterMatch.captured(1);
            QString argument = getTextArgument(
                    specialFilterMatch.captured(2), &tokens)
                                       .trimmed();
            if (!argument.isEmpty()) {
                if (field == "key") {
                    mixxx::track::io::key::ChromaticKey key =
                            KeyUtils::guessKeyFromText(argument);
                    if (key == mixxx::track::io::key::INVALID) {
                        if (argument == kMissingFieldSearchTerm) {
                            pNode = std::make_unique<NullOrEmptyTextFilterNode>(
                                    m_pTrackCollection->database(), m_fieldToSqlColumns[field]);
                        } else {
                            pNode = std::make_unique<TextFilterNode>(
                                    m_pTrackCollection->database(), m_fieldToSqlColumns[field], argument);
                        }
                    } else {
                        pNode = std::make_unique<KeyFilterNode>(key, fuzzy);
                    }
                } else if (field == "duration") {
                    pNode = std::make_unique<DurationFilterNode>(
                            m_fieldToSqlColumns[field], argument);
                } else if (field == "date_added" ||
                           field == "datetime_added" ||
                           field == "added" ||
                           field == "dateadded") {
                    field = "datetime_added";
                    pNode = std::make_unique<TextFilterNode>(
                        m_pTrackCollection->database(), m_fieldToSqlColumns[field], argument);
                }
            }
        } else {
            // If no advanced search feature matched, treat it as a search term.
            if (negate) {
                token = token.mid(1);
            }
            // Don't trigger on a lone minus sign.
            if (!token.isEmpty()) {
                QString argument = getTextArgument(token, &tokens);
                // For untagged strings we search the track fields as well
                // as the crate names the track is in. This allows the user
                // to use crates like tags
                if (searchColumns.contains("crate")) {
                    std::unique_ptr<OrNode> gNode = std::make_unique<OrNode>();

                    gNode->addNode(std::make_unique<CrateFilterNode>(
                                    &m_pTrackCollection->crates(), argument));
                    gNode->addNode(std::make_unique<TextFilterNode>(
                                    m_pTrackCollection->database(), queryColumns, argument));

                    pNode = std::move(gNode);
                } else {
                    pNode = std::make_unique<TextFilterNode>(
                             m_pTrackCollection->database(), queryColumns, argument);
                }
            }
        }
        if (pNode) {
            if (negate) {
                pNode = std::make_unique<NotNode>(std::move(pNode));
            }
            pQuery->addNode(std::move(pNode));
        }
    }
}

std::unique_ptr<QueryNode> SearchQueryParser::parseQuery(const QString& query,
                                         const QStringList& searchColumns,
                                         const QString& extraFilter) const {
    auto pQuery(std::make_unique<AndNode>());

    if (!extraFilter.isEmpty()) {
        pQuery->addNode(std::make_unique<SqlNode>(extraFilter));
    }

    if (!query.isEmpty()) {
        QStringList tokens = query.split(" ");
        parseTokens(tokens, searchColumns, pQuery.get());
    }

    return pQuery;
}

QStringList SearchQueryParser::splitQueryIntoWords(const QString& query) {
    QStringList queryWordList = query.split(kSplitIntoWordsRegexp,
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            Qt::SkipEmptyParts);
#else
            QString::SkipEmptyParts);
#endif
    return queryWordList;
}

bool SearchQueryParser::queryIsLessSpecific(const QString& original, const QString& changed) {
    // separate search query into tokens
    QStringList oldWordList = SearchQueryParser::splitQueryIntoWords(original);
    QStringList newWordList = SearchQueryParser::splitQueryIntoWords(changed);

    // we sort the lists for length so the comperator will pop the longest match first
    std::sort(oldWordList.begin(), oldWordList.end(), [=](const QString& v1, const QString& v2) {
        return v1.length() > v2.length();
    });
    std::sort(newWordList.begin(), newWordList.end(), [=](const QString& v1, const QString& v2) {
        return v1.length() > v2.length();
    });

    for (int i = 0; i < oldWordList.length(); i++) {
        const QString& oldWord = oldWordList.at(i);
        for (int j = 0; j < newWordList.length(); j++) {
            const QString& newWord = newWordList.at(j);
            // Note(ronso0) Look for missing '~' in newWord (fuzzy matching)?
            if ((oldWord.startsWith("-") && oldWord.startsWith(newWord)) ||
                    (!newWord.contains(":") && oldWord.contains(newWord)) ||
                    (newWord.contains(":") && oldWord.startsWith(newWord))) {
                // we found a match and can remove the search term list
                newWordList.removeAt(j);
                break;
            }
        }
    }
    // if the new search query list contains no more terms, we have a reduced
    // search term
    if (newWordList.empty()) {
        return true;
    }
    return false;
}
