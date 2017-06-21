#include <QtDebug>

#include "library/searchquery.h"

#include "library/queryutil.h"
#include "track/keyutils.h"
#include "library/dao/trackschema.h"
#include "util/db/sqllikewildcards.h"

QVariant getTrackValueForColumn(const TrackPointer& pTrack, const QString& column) {
    if (column == LIBRARYTABLE_ARTIST) {
        return pTrack->getArtist();
    } else if (column == LIBRARYTABLE_TITLE) {
        return pTrack->getTitle();
    } else if (column == LIBRARYTABLE_ALBUM) {
        return pTrack->getAlbum();
    } else if (column == LIBRARYTABLE_ALBUMARTIST) {
        return pTrack->getAlbumArtist();
    } else if (column == LIBRARYTABLE_YEAR) {
        return pTrack->getYear();
    } else if (column == LIBRARYTABLE_DATETIMEADDED) {
        return pTrack->getDateAdded();
    } else if (column == LIBRARYTABLE_GENRE) {
        return pTrack->getGenre();
    } else if (column == LIBRARYTABLE_COMPOSER) {
        return pTrack->getComposer();
    } else if (column == LIBRARYTABLE_GROUPING) {
        return pTrack->getGrouping();
    } else if (column == LIBRARYTABLE_FILETYPE) {
        return pTrack->getType();
    } else if (column == LIBRARYTABLE_TRACKNUMBER) {
        return pTrack->getTrackNumber();
    } else if (column == LIBRARYTABLE_LOCATION) {
        return QDir::toNativeSeparators(pTrack->getLocation());
    } else if (column == LIBRARYTABLE_COMMENT) {
        return pTrack->getComment();
    } else if (column == LIBRARYTABLE_DURATION) {
        return pTrack->getDuration();
    } else if (column == LIBRARYTABLE_BITRATE) {
        return pTrack->getBitrate();
    } else if (column == LIBRARYTABLE_BPM) {
        return pTrack->getBpm();
    } else if (column == LIBRARYTABLE_PLAYED) {
        return pTrack->getPlayCounter().isPlayed();
    } else if (column == LIBRARYTABLE_TIMESPLAYED) {
        return pTrack->getPlayCounter().getTimesPlayed();
    } else if (column == LIBRARYTABLE_RATING) {
        return pTrack->getRating();
    } else if (column == LIBRARYTABLE_KEY) {
        return pTrack->getKeyText();
    } else if (column == LIBRARYTABLE_KEY_ID) {
        return static_cast<int>(pTrack->getKey());
    } else if (column == LIBRARYTABLE_BPM_LOCK) {
        return pTrack->isBpmLocked();
    }

    return QVariant();
}

//static
QString QueryNode::concatSqlClauses(
        const QStringList& sqlClauses, const QString& sqlConcatOp) {
    switch (sqlClauses.size()) {
    case 0:
        return QString();
    case 1:
        return sqlClauses.front();
    default:
        // The component terms need to be wrapped into parantheses,
        // but the whole expression does not. The composite node is
        // always responsible for proper wrapping into parantheses!
        return "(" % sqlClauses.join(") " % sqlConcatOp % " (") % ")";
    }
}

bool AndNode::match(const TrackPointer& pTrack) const {
    for (const auto& pNode: m_nodes) {
        if (!pNode->match(pTrack)) {
            return false;
        }
    }
    // An empty AND node always evaluates to true! This
    // is consistent with the generated SQL query.
    return true;
}

QString AndNode::toSql() const {
    QStringList queryFragments;
    queryFragments.reserve(m_nodes.size());
    for (const auto& pNode: m_nodes) {
        QString sql = pNode->toSql();
        if (!sql.isEmpty()) {
            queryFragments << sql;
        }
    }
    return concatSqlClauses(queryFragments, "AND");
}

bool OrNode::match(const TrackPointer& pTrack) const {
    // An empty OR node would always evaluate to false
    // which is inconsistent with the generated SQL query!
    VERIFY_OR_DEBUG_ASSERT(!m_nodes.empty()) {
        // Evaluate to true even if the correct choice would
        // be false to keep the evaluation consistent with
        // the generated SQL query.
        return true;
    }
    for (const auto& pNode: m_nodes) {
        if (pNode->match(pTrack)) {
            return true;
        }
    }
    return false;
}

QString OrNode::toSql() const {
    QStringList queryFragments;
    queryFragments.reserve(m_nodes.size());
    for (const auto& pNode: m_nodes) {
        QString sql = pNode->toSql();
        if (!sql.isEmpty()) {
            queryFragments << sql;
        }
    }
    return concatSqlClauses(queryFragments, "OR");
}

bool NotNode::match(const TrackPointer& pTrack) const {
    return !m_pNode->match(pTrack);
}

QString NotNode::toSql() const {
    QString sql(m_pNode->toSql());
    if (sql.isEmpty()) {
        return QString();
    } else {
        // The component term needs to be wrapped into parantheses,
        // but the whole expression does not. The composite node is
        // always responsible for proper wrapping into parantheses!
        return "NOT (" % sql % ")";
    }
}

bool TextFilterNode::match(const TrackPointer& pTrack) const {
    for (const auto& sqlColumn: m_sqlColumns) {
        QVariant value = getTrackValueForColumn(pTrack, sqlColumn);
        if (!value.isValid() || !qVariantCanConvert<QString>(value)) {
            continue;
        }

        if (value.toString().contains(m_argument, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

QString TextFilterNode::toSql() const {
    FieldEscaper escaper(m_database);
    QString escapedArgument = escaper.escapeString(kSqlLikeMatchAll + m_argument + kSqlLikeMatchAll);

    QStringList searchClauses;
    for (const auto& sqlColumn: m_sqlColumns) {
        searchClauses << QString("%1 LIKE %2").arg(sqlColumn, escapedArgument);
    }
    return concatSqlClauses(searchClauses, "OR");
}

CrateFilterNode::CrateFilterNode(const CrateStorage* pCrateStorage,
                                 const QString& crateNameLike)
    : m_pCrateStorage(pCrateStorage),
      m_crateNameLike(crateNameLike),
      m_matchInitialized(false) {
}

bool CrateFilterNode::match(const TrackPointer& pTrack) const {
    if (!m_matchInitialized) {
        CrateTrackSelectResult crateTracks(
             m_pCrateStorage->selectTracksSortedByCrateNameLike(m_crateNameLike));

        while (crateTracks.next()) {
            m_matchingTrackIds.push_back(crateTracks.trackId());
        }

        m_matchInitialized = true;
    }

    return std::binary_search(m_matchingTrackIds.begin(), m_matchingTrackIds.end(), pTrack->getId());
}

QString CrateFilterNode::toSql() const {
    return QString("id IN (%1)").arg(CrateStorage::formatQueryForTrackIdsByCrateNameLike(m_crateNameLike));
}

NumericFilterNode::NumericFilterNode(const QStringList& sqlColumns)
        : m_sqlColumns(sqlColumns),
          m_bOperatorQuery(false),
          m_operator("="),
          m_dOperatorArgument(0.0),
          m_bRangeQuery(false),
          m_dRangeLow(0.0),
          m_dRangeHigh(0.0) {
}

NumericFilterNode::NumericFilterNode(
        const QStringList& sqlColumns, const QString& argument)
        : NumericFilterNode(sqlColumns) {
    init(argument);
}

void NumericFilterNode::init(QString argument) {
    QRegExp operatorMatcher("^(>|>=|=|<|<=)(.*)$");
    if (operatorMatcher.indexIn(argument) != -1) {
        m_operator = operatorMatcher.cap(1);
        argument = operatorMatcher.cap(2);
    }

    bool parsed = false;
    // Try to convert to see if it parses.
    m_dOperatorArgument = parse(argument, &parsed);
    if (parsed) {
        m_bOperatorQuery = true;
    }

    QStringList rangeArgs = argument.split("-");
    if (rangeArgs.length() == 2) {
        bool lowOk = false;
        m_dRangeLow = parse(rangeArgs[0], &lowOk);
        bool highOk = false;
        m_dRangeHigh = parse(rangeArgs[1], &highOk);

        if (lowOk && highOk && m_dRangeLow <= m_dRangeHigh) {
            m_bRangeQuery = true;
        }
    }
}

double NumericFilterNode::parse(const QString& arg, bool *ok) {
    return arg.toDouble(ok);
}

bool NumericFilterNode::match(const TrackPointer& pTrack) const {
    for (const auto& sqlColumn: m_sqlColumns) {
        QVariant value = getTrackValueForColumn(pTrack, sqlColumn);
        if (!value.isValid() || !qVariantCanConvert<double>(value)) {
            continue;
        }

        double dValue = value.toDouble();
        if (m_bOperatorQuery) {
            if ((m_operator == "=" && dValue == m_dOperatorArgument) ||
                (m_operator == "<" && dValue < m_dOperatorArgument) ||
                (m_operator == ">" && dValue > m_dOperatorArgument) ||
                (m_operator == "<=" && dValue <= m_dOperatorArgument) ||
                (m_operator == ">=" && dValue >= m_dOperatorArgument)) {
                return true;
            }
        } else if (m_bRangeQuery && dValue >= m_dRangeLow &&
                   dValue <= m_dRangeHigh) {
            return true;
        }
    }
    return false;
}

QString NumericFilterNode::toSql() const {
    if (m_bOperatorQuery) {
        QStringList searchClauses;
        for (const auto& sqlColumn: m_sqlColumns) {
            searchClauses << QString("%1 %2 %3").arg(
                sqlColumn, m_operator, QString::number(m_dOperatorArgument));
        }
        return concatSqlClauses(searchClauses, "OR");
    }

    if (m_bRangeQuery) {
        QStringList searchClauses;
        for (const auto& sqlColumn: m_sqlColumns) {
            QStringList rangeClauses;
            rangeClauses << QString("%1 >= %2").arg(
                    sqlColumn, QString::number(m_dRangeLow));
            rangeClauses << QString("%1 <= %2").arg(
                    sqlColumn, QString::number(m_dRangeHigh));
            searchClauses << concatSqlClauses(rangeClauses, "AND");
        }
        return concatSqlClauses(searchClauses, "OR");
    }

    return QString();
}

DurationFilterNode::DurationFilterNode(
        const QStringList& sqlColumns, const QString& argument)
        : NumericFilterNode(sqlColumns) {
    // init() has to be called from this class directly to invoke
    // the implementation of this and not that of the base class!
    init(argument);
}

double DurationFilterNode::parse(const QString& arg, bool* ok) {
    QRegExp regex("^(\\d*)(m|:)?([0-6]?\\d)?s?$");
    if (regex.indexIn(arg) == -1) {
        *ok = false;
        return 0;
    }

    // You can check that the minutes are parsed to entry 2 of the list and the
    // seconds are in the 4th entry. If you don't believe me or this doesn't
    // work anymore because we changed our Qt version just have a look at caps.
    // -- (kain88, Aug 2014)
    QStringList caps = regex.capturedTexts();
    double m = 0;
    double s = 0;
    // if only a number is entered parse as seconds
    if (caps.at(3).isEmpty() && caps.at(2).isEmpty()) {
        s = caps.at(1).toDouble(ok);
    } else {
        m = caps.at(1).toDouble(ok);
        s = caps.at(3).toDouble();
    }

    if (!*ok) {
        return 0;
    }

    *ok = true;
    return 60 * m + s;
}

KeyFilterNode::KeyFilterNode(mixxx::track::io::key::ChromaticKey key,
                             bool fuzzy) {
    if (fuzzy) {
        m_matchKeys = KeyUtils::getCompatibleKeys(key);
    } else {
        m_matchKeys.push_back(key);
    }
}

bool KeyFilterNode::match(const TrackPointer& pTrack) const {
    return m_matchKeys.contains(pTrack->getKey());
}

QString KeyFilterNode::toSql() const {
    QStringList searchClauses;
    for (const auto& matchKey: m_matchKeys) {
        searchClauses << QString("key_id IS %1").arg(QString::number(matchKey));
    }
    return concatSqlClauses(searchClauses, "OR");
}
