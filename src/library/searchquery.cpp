#include "library/searchquery.h"

#include <QRegularExpression>

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackset/crate/crateschema.h"
#include "library/trackset/crate/cratestorage.h" // for CrateTrackSelectResult
#include "track/keyutils.h"
#include "track/track.h"
#include "util/db/dbconnection.h"
#include "util/db/sqllikewildcards.h"

namespace {

constexpr double kLibraryRoundRange = 0.05;

const QRegularExpression kDurationRegex(QStringLiteral("^(\\d+)(m|:)?([0-5]?\\d)?s?$"));

// The ordering of operator alternatives separated by '|' is crucial to avoid incomplete
// partial matches, e.g. by capturing "<" + "=" + <arg>  instead of "<=" + <arg>!
//
// See also: https://perldoc.perl.org/perlre
// > Alternatives are tried from left to right, so the first alternative found for which
// > the entire expression matches, is the one that is chosen. This means that alternatives
// > are not necessarily greedy.
const QRegularExpression kNumericOperatorRegex(QStringLiteral("^(<=|>=|=|<|>)(.*)$"));

const QRegularExpression kNullRegex(QStringLiteral("^([0.,]+)$"));

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
        // We use only the year that is part of the first four digits
        // In all possible formats.
        return pTrack->getYear().left(4);
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
    } else if (column == TRACKLOCATIONSTABLE_LOCATION) {
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
    } else if (column == LIBRARYTABLE_LAST_PLAYED_AT) {
        return pTrack->getLastPlayedAt();
    } else if (column == LIBRARYTABLE_RATING) {
        return pTrack->getRating();
    } else if (column == LIBRARYTABLE_KEY) {
        return pTrack->getKeyText();
    } else if (column == LIBRARYTABLE_KEY_ID) {
        return static_cast<int>(pTrack->getKey());
    } else if (column == LIBRARYTABLE_BPM_LOCK) {
        return pTrack->isBpmLocked();
    } else if (column == LIBRARYTABLE_ID) {
        return pTrack->getId().toVariant();
    }

    return QVariant();
}

QString concatSqlClauses(
        const QStringList& sqlClauses, const QString& sqlConcatOp) {
    switch (sqlClauses.size()) {
    case 0:
        return QString();
    case 1:
        return sqlClauses.front();
    default:
        // The component terms need to be wrapped into parentheses,
        // but the whole expression does not. The composite node is
        // always responsible for proper wrapping into parentheses!
        //        qDebug() << "[SEARCHQUERY] [concatSqlClauses] -> sqlConcatOp = " << sqlConcatOp;
        //        qDebug() << "[SEARCHQUERY] [concatSqlClauses] -> sqlClauses = " << sqlClauses;
        return QChar('(') +
                sqlClauses.join(
                        QStringLiteral(") ") + sqlConcatOp + QStringLiteral(" (")) +
                QChar(')');
    }
}

} // namespace

bool AndNode::match(const TrackPointer& pTrack) const {
    for (const auto& pNode : m_nodes) {
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
    queryFragments.reserve(static_cast<int>(m_nodes.size()));
    for (const auto& pNode : m_nodes) {
        QString sql = pNode->toSql();
        if (!sql.isEmpty()) {
            queryFragments << sql;
        }
    }
    //    qDebug() << "[SEARCHQUERY] [AndNode::toSql()] -> queryFragments = " << queryFragments;
    return concatSqlClauses(queryFragments, "AND");
}

bool OrNode::match(const TrackPointer& pTrack) const {
    for (const auto& pNode : m_nodes) {
        if (pNode->match(pTrack)) {
            return true;
        }
    }
    return false;
}

QString OrNode::toSql() const {
    if (m_nodes.empty()) {
        return "FALSE";
    }
    QStringList queryFragments;
    queryFragments.reserve(static_cast<int>(m_nodes.size()));
    for (const auto& pNode : m_nodes) {
        QString sql = pNode->toSql();
        if (!sql.isEmpty()) {
            queryFragments << sql;
        }
    }
    //    qDebug() << "[SEARCHQUERY] [OrNode::toSql()] -> queryFragments = " << queryFragments;
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
        // The component term needs to be wrapped into parentheses,
        // but the whole expression does not. The composite node is
        // always responsible for proper wrapping into parentheses!
        //        qDebug() << "[SEARCHQUERY] [NotNode::toSql()] -> return: "
        //                 << "NOT (" % sql % ")";
        return "NOT (" % sql % ")";
    }
}

TextFilterNode::TextFilterNode(const QSqlDatabase& database,
        const QStringList& sqlColumns,
        const QString& argument,
        const StringMatch matchMode)
        : m_database(database),
          m_sqlColumns(sqlColumns),
          m_argument(argument),
          m_matchMode(matchMode) {
    mixxx::DbConnection::makeStringLatinLow(&m_argument);
}

bool TextFilterNode::match(const TrackPointer& pTrack) const {
    for (const auto& sqlColumn : m_sqlColumns) {
        QVariant value = getTrackValueForColumn(pTrack, sqlColumn);
        if (!value.isValid() || !value.canConvert<QString>()) {
            continue;
        }

        QString strValue = value.toString();
        mixxx::DbConnection::makeStringLatinLow(&strValue);
        if (m_matchMode == StringMatch::Equals) {
            if (strValue == m_argument) {
                return true;
            }
        } else {
            if (strValue.contains(m_argument)) {
                return true;
            }
        }
    }
    return false;
}

QString TextFilterNode::toSql() const {
    FieldEscaper escaper(m_database);
    QString argument = m_argument;
    if (argument.size() > 0) {
        if (argument[argument.size() - 1].isSpace()) {
            // LIKE eats a trailing space. This can be avoided by adding a '_'
            // as a delimiter that matches any following character.
            argument.append('_');
        }
    }
    QString escapedArgument;
    // Using a switch-case without default case to get a compile-time -Wswitch warning
    switch (m_matchMode) {
    case StringMatch::Contains:
        escapedArgument = escaper.escapeString(
                kSqlLikeMatchAll + argument + kSqlLikeMatchAll);
        break;
    case StringMatch::Equals:
        escapedArgument = escaper.escapeString(argument);
        break;
    }
    QStringList searchClauses;
    for (const auto& sqlColumn : m_sqlColumns) {
        searchClauses << QString("%1 LIKE %2").arg(sqlColumn, escapedArgument);
        //        qDebug() << "[SEARCHQUERY] [TextFilterNode::toSql()] ->
        //        sqlColumn " << sqlColumn; qDebug() << "[SEARCHQUERY]
        //        [TextFilterNode::toSql()] -> escapedArgument " <<
        //        escapedArgument; qDebug() << "[SEARCHQUERY]
        //        [TextFilterNode::toSql()] -> searchClauses " << searchClauses;
    }
    return concatSqlClauses(searchClauses, "OR");
}

bool NullOrEmptyTextFilterNode::match(const TrackPointer& pTrack) const {
    if (!m_sqlColumns.isEmpty()) {
        // only use the major column
        QVariant value = getTrackValueForColumn(pTrack, m_sqlColumns.first());
        if (!value.isValid() || !value.canConvert<QString>()) {
            return true;
        }
        return value.toString().isEmpty();
    }
    return false;
}

QString NullOrEmptyTextFilterNode::toSql() const {
    if (!m_sqlColumns.isEmpty()) {
        // only use the major column
        return QString("%1 IS NULL OR %1 IS ''").arg(m_sqlColumns.first());
    }
    return QString();
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
    return QString("id IN (%1)")
            .arg(m_pCrateStorage->formatQueryForTrackIdsByCrateNameLike(
                    m_crateNameLike));
}

NoCrateFilterNode::NoCrateFilterNode(const CrateStorage* pCrateStorage)
        : m_pCrateStorage(pCrateStorage),
          m_matchInitialized(false) {
}

bool NoCrateFilterNode::match(const TrackPointer& pTrack) const {
    if (!m_matchInitialized) {
        TrackSelectResult tracks(
                m_pCrateStorage->selectAllTracksSorted());

        while (tracks.next()) {
            m_matchingTrackIds.push_back(tracks.trackId());
        }

        m_matchInitialized = true;
    }

    return !std::binary_search(m_matchingTrackIds.begin(), m_matchingTrackIds.end(), pTrack->getId());
}

QString NoCrateFilterNode::toSql() const {
    return QString("%1 NOT IN (%2)")
            .arg(CRATETABLE_ID,
                    CrateStorage::formatQueryForTrackIdsWithCrate());
}

NumericFilterNode::NumericFilterNode(const QStringList& sqlColumns)
        : m_sqlColumns(sqlColumns),
          m_bOperatorQuery(false),
          m_bNullQuery(false),
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
    if (argument == kMissingFieldSearchTerm) {
        m_bNullQuery = true;
        return;
    }

    QRegularExpressionMatch match = kNumericOperatorRegex.match(argument);
    if (match.hasMatch()) {
        m_operator = match.captured(1);
        argument = match.captured(2);
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

double NumericFilterNode::parse(const QString& arg, bool* ok) {
    return arg.toDouble(ok);
}

bool NumericFilterNode::match(const TrackPointer& pTrack) const {
    for (const auto& sqlColumn : m_sqlColumns) {
        QVariant value = getTrackValueForColumn(pTrack, sqlColumn);
        if (!value.isValid() || !value.canConvert<double>()) {
            if (m_bNullQuery) {
                return true;
            }
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
    if (m_bNullQuery) {
        for (const auto& sqlColumn : m_sqlColumns) {
            // only use the major column
            return QString("%1 IS NULL").arg(sqlColumn);
        }
        return QString();
    }

    if (m_bOperatorQuery) {
        QStringList searchClauses;
        for (const auto& sqlColumn : m_sqlColumns) {
            searchClauses << QString("%1 %2 %3")
                                     .arg(sqlColumn,
                                             m_operator,
                                             QString::number(
                                                     m_dOperatorArgument));
        }
        return concatSqlClauses(searchClauses, "OR");
    }

    if (m_bRangeQuery) {
        QStringList searchClauses;
        for (const auto& sqlColumn : m_sqlColumns) {
            searchClauses << QString(QStringLiteral("%1 BETWEEN %2 AND %3"))
                                     .arg(sqlColumn,
                                             QString::number(m_dRangeLow),
                                             QString::number(m_dRangeHigh));
            //            qDebug() << "[SEARCHQUERY]
            //            [NumericFilterNode::toSql()] -> searchClauses " <<
            //            searchClauses;
        }
        return concatSqlClauses(searchClauses, "OR");
    }

    return QString();
}

NullNumericFilterNode::NullNumericFilterNode(const QStringList& sqlColumns)
        : m_sqlColumns(sqlColumns) {
}

bool NullNumericFilterNode::match(const TrackPointer& pTrack) const {
    if (!m_sqlColumns.isEmpty()) {
        // only use the major column
        QVariant value = getTrackValueForColumn(pTrack, m_sqlColumns.first());
        if (!value.isValid() || !value.canConvert<double>()) {
            return true;
        }
    }
    return false;
}

QString NullNumericFilterNode::toSql() const {
    if (!m_sqlColumns.isEmpty()) {
        // only use the major column
        return QString("%1 IS NULL").arg(m_sqlColumns.first());
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
    QRegularExpressionMatch match = kDurationRegex.match(arg);
    if (!match.hasMatch()) {
        *ok = false;
        return 0;
    }

    // You can check that the minutes are parsed to entry 2 of the list and the
    // seconds are in the 4th entry. If you don't believe me or this doesn't
    // work anymore because we changed our Qt version just have a look at caps.
    // -- (kain88, Aug 2014)
    double m = 0;
    double s = 0;
    // if only a number is entered parse as seconds
    if (match.captured(3).isEmpty() && match.captured(2).isEmpty()) {
        s = match.captured(1).toDouble(ok);
    } else {
        m = match.captured(1).toDouble(ok);
        s = match.captured(3).toDouble();
    }

    if (!*ok) {
        return 0;
    }

    *ok = true;
    return 60 * m + s;
}

// static
constexpr double BpmFilterNode::kRelativeRangeDefault;

// static
double BpmFilterNode::s_relativeRange = kRelativeRangeDefault;

// static
void BpmFilterNode::setBpmRelativeRange(double range) {
    // range < 0 would yield zero results because m_dRangeLow > m_dRangeHigh
    VERIFY_OR_DEBUG_ASSERT(range >= 0) {
        return;
    }
    s_relativeRange = range;
}

namespace {

inline QString rangeSqlString(double lower, double upper) {
    // 'BETWEEN' is inclusive, i.e. returns true if lower <= value <= upper
    return QStringLiteral("bpm BETWEEN %1 AND %2")
            .arg(QString::number(lower),
                    QString::number(upper));
}

inline QString rangeUpperExclusiveSqlString(double lower, double upper) {
    return QStringLiteral("bpm >= %1 AND bpm < %2")
            .arg(QString::number(lower),
                    QString::number(upper));
}

inline std::pair<double, double> rangeFromTrailingDecimal(double bpm) {
    // Set up a range if we have decimals. This will include matches
    // for which we show rounded values in the library. For example
    // 124.0  finds 123.95 - 124.05
    // 124.1  finds 124.05 - 124.15
    // 124.92 finds 124.915 - 124.925
    if (bpm == 0.0) {
        return std::pair<double, double>(bpm, bpm);
    }
    int numDecimals = 1;
    double intPart;
    double fractPart = std::modf(bpm, &intPart);
    if (fractPart != 0.0) {
        QString decimals = QString::number(fractPart);
        numDecimals = decimals.split('.').at(1).length();
    }
    double roundRange = 5 / pow(10, numDecimals + 1);
    // Don't search for negative BPM
    double lower = std::max(0.0, bpm - roundRange);
    double upper = bpm + roundRange;
    return std::pair<double, double>(lower, upper);
}

} // namespace

BpmFilterNode::BpmFilterNode(QString& argument, bool fuzzy, bool negate)
        : m_matchMode(MatchMode::Invalid),
          m_operator("="),
          m_bpm(0.0),
          m_rangeLower(0.0),
          m_rangeUpper(0.0),
          m_bpmHalfLower(0.0),
          m_bpmHalfUpper(0.0),
          m_bpmDoubleLower(0.0),
          m_bpmDoubleUpper(0.0) {
    QRegularExpressionMatch nullMatch = kNullRegex.match(argument);
    if (argument == kMissingFieldSearchTerm || // explicit empty
            argument == "-" ||                 // displayed in the BPM column
            nullMatch.hasMatch()) {            // displayed in the BPM widgets
        m_matchMode = MatchMode::Null;
        return;
    }

    QRegularExpressionMatch opMatch = kNumericOperatorRegex.match(argument);
    if (opMatch.hasMatch()) {
        if (fuzzy) {
            // fuzzy can't be combined with operators
            // m_matchMode is already Invalid.
            return;
        }
        m_operator = opMatch.captured(1);
        argument = opMatch.captured(2);
    }

    // Replace the locale's decimal separator with .
    // This is handy if numbers are typed with the numpad.
    argument.replace(',', '.');
    bool isDouble = false;
    double bpm = argument.toDouble(&isDouble);
    if (isDouble) {
        // Check if the arg has a decimal separator (even if no digits after that)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        bool strictMatch = argument.split('.', Qt::SkipEmptyParts).length() > 1;
#else
        bool strictMatch = argument.split('.', QString::SkipEmptyParts).length() > 1;
#endif
        if (fuzzy) {
            // fuzzy search +- n%
            m_matchMode = MatchMode::Fuzzy;
        } else if (!opMatch.hasMatch() && !negate) {
            // Simple 'bpm:NNN' search.
            // Also searches for half/double matches (rounded up/down)
            // Center value is turned into range in order to ...
            if (strictMatch) {
                // find rounded values (hiddendecimals in tracks table / BPM widgets)
                m_matchMode = MatchMode::HalveDoubleStrict;
            } else {
                // find all values with same int base
                m_matchMode = MatchMode::HalveDouble;
            }
        } else {
            // Operator or basic query, optional negate
            if (m_operator == '=') {
                // Explicit search.
                // Same center range as HalveDouble/~Strict
                // TODO What about -bpm: ? ExplicitNot
                if (strictMatch) {
                    m_matchMode = MatchMode::ExplicitStrict;
                } else {
                    m_matchMode = MatchMode::Explicit;
                }
            } else {
                m_matchMode = MatchMode::Operator;
            }
        }
    } else {
        if (fuzzy) {
            // fuzzy can't be combined with range
            return;
        }
        // Test if this is a valid range query
        QStringList rangeArgs = argument.split("-");
        if (rangeArgs.length() == 2) {
            bool lowOk = false;
            bool highOk = false;
            double rangeLower = rangeArgs[0].toDouble(&lowOk);
            double rangeUpper = rangeArgs[1].toDouble(&highOk);

            if (lowOk && highOk && rangeLower <= rangeUpper) {
                // Assign values now to avoid moving bounds out of scope
                m_matchMode = MatchMode::Range;
                m_rangeLower = rangeLower;
                m_rangeUpper = rangeUpper;
                return;
            }
        }
        // m_matchMode is already Invalid.
        return;
    }

    // Build/prepare match functions
    switch (m_matchMode) {
    case MatchMode::Explicit: {
        // No decimals: 114 finds   113.95 - 114.99999
        m_rangeLower = rangeFromTrailingDecimal(bpm).first;
        m_rangeUpper = floor(bpm + 1);
        break;
    }
    case MatchMode::ExplicitStrict: {
        // Decimals: 114.0 finds 113.95 - 114.05
        std::tie(m_rangeLower, m_rangeUpper) = rangeFromTrailingDecimal(bpm);
        break;
    }
    case MatchMode::Fuzzy: {
        m_rangeLower = floor((1 - s_relativeRange) * bpm);
        m_rangeUpper = ceil((1 + s_relativeRange) * bpm);
        break;
    }
    case MatchMode::HalveDouble: {
        DEBUG_ASSERT(bpm == floor(bpm));
        // For instance bpm = 127
        // We find everything that is displayed as 127.** in the library
        m_rangeLower = bpm - kLibraryRoundRange; // [126.95
        m_rangeUpper = bpm + 1;                  // ]128

        double halfBpm = bpm / 2;
        if (floor(halfBpm) != halfBpm) {
            // In case bpm / 2 is a fractional, we include the *.0 value neighbours
            // since fractional bpm values are less common in some genres.
            m_bpmHalfLower = floor(halfBpm) - kLibraryRoundRange; // [62.95
        } else {
            // Here we are for instance with 126
            // We find everything that is displayed as 63.** in the library
            m_bpmHalfLower = halfBpm - kLibraryRoundRange; //        [62.95
        }
        m_bpmHalfUpper = halfBpm + 1; //                                ]64

        // We find everything which would be found when editing the beat grid via / 2
        m_bpmDoubleLower = (bpm * 2) - kLibraryRoundRange; // [253.95
        m_bpmDoubleUpper = m_rangeUpper * 2;               // ]256
        break;
    }
    case MatchMode::HalveDoubleStrict: { //            57.0
        std::tie(m_rangeLower, m_rangeUpper) =
                rangeFromTrailingDecimal(bpm); //      56.95 - 57.05
        std::tie(m_bpmHalfLower, m_bpmHalfUpper) =
                rangeFromTrailingDecimal(bpm / 2); //  28.45 - 28.55
        std::tie(m_bpmDoubleLower, m_bpmDoubleUpper) =
                rangeFromTrailingDecimal(bpm * 2); // 113.95 - 114.05
        break;
    }
    case MatchMode::Operator: {
        m_bpm = bpm;
        break;
    }
    default:
        return;
    }
}

bool BpmFilterNode::match(const TrackPointer& pTrack) const {
    double value = pTrack->getBpm();

    switch (m_matchMode) {
    case MatchMode::Null: {
        return value == 0.0;
    }
    case MatchMode::Explicit: {
        return value >= m_rangeLower && value < m_rangeUpper;
    }
    case MatchMode::ExplicitStrict:
    case MatchMode::Fuzzy:
    case MatchMode::Range: {
        return value >= m_rangeLower && value <= m_rangeUpper;
    }
    case MatchMode::HalveDouble: {
        return (value >= m_rangeLower && value <= m_rangeUpper) ||
                (value >= m_bpmHalfLower && value <= m_bpmHalfUpper) ||
                (value >= m_bpmDoubleLower && value <= m_bpmDoubleUpper);
    }
    case MatchMode::HalveDoubleStrict: {
        return (value >= m_rangeLower && value < m_rangeUpper) ||
                (value >= m_bpmHalfLower && value <= m_bpmHalfUpper) ||
                (value >= m_bpmDoubleLower && value <= m_bpmDoubleUpper);
    }
    case MatchMode::Operator: {
        return (m_operator == "=" && value == m_bpm) ||
                (m_operator == "<" && value < m_bpm) ||
                (m_operator == ">" && value > m_bpm) ||
                (m_operator == "<=" && value <= m_bpm) ||
                (m_operator == ">=" && value >= m_bpm);
    }
    default: // e.g. MatchMode::Invalid
        // Show no results to indicate the query is invalid.
        // Is this good UX?
        return false;
    }
}

QString BpmFilterNode::toSql() const {
    switch (m_matchMode) {
    case MatchMode::Null: {
        return QString("bpm IS 0");
    }
    case MatchMode::Explicit: {
        return QStringLiteral("bpm >= %1 AND bpm < %2")
                .arg(QString::number(m_rangeLower),
                        QString::number(m_rangeUpper));
    }
    case MatchMode::ExplicitStrict:
    case MatchMode::Fuzzy:
    case MatchMode::Range: {
        return rangeSqlString(m_rangeLower, m_rangeUpper);
    }
    case MatchMode::HalveDouble: {
        QStringList searchClauses;
        searchClauses << rangeUpperExclusiveSqlString(m_rangeLower, m_rangeUpper);
        searchClauses << rangeUpperExclusiveSqlString(m_bpmHalfLower, m_bpmHalfUpper);
        searchClauses << rangeUpperExclusiveSqlString(m_bpmDoubleLower, m_bpmDoubleUpper);
        return concatSqlClauses(searchClauses, "OR");
    }
    case MatchMode::HalveDoubleStrict: {
        QStringList searchClauses;
        searchClauses << rangeSqlString(m_rangeLower, m_rangeUpper);
        searchClauses << rangeSqlString(m_bpmHalfLower, m_bpmHalfUpper);
        searchClauses << rangeSqlString(m_bpmDoubleLower, m_bpmDoubleUpper);
        return concatSqlClauses(searchClauses, "OR");
    }
    case MatchMode::Operator: {
        return QString("bpm %1 %2").arg(m_operator, QString::number(m_bpm));
    }
    default: // MatchMode::Invalid
        return QString("bpm IS NULL");
    }
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
    for (const auto& matchKey : m_matchKeys) {
        searchClauses << QString("key_id IS %1").arg(QString::number(matchKey));
    }
    return concatSqlClauses(searchClauses, "OR");
}

YearFilterNode::YearFilterNode(
        const QStringList& sqlColumns, const QString& argument)
        : NumericFilterNode(sqlColumns, argument) {
}

QString YearFilterNode::toSql() const {
    if (m_bNullQuery) {
        return QStringLiteral("year IS NULL");
    }

    if (m_bOperatorQuery) {
        return QString(
                QStringLiteral("CAST(substr(year,1,4) AS INTEGER) %1 %2"))
                .arg(m_operator, QString::number(m_dOperatorArgument));
    }

    if (m_bRangeQuery) {
        return QString(
                QStringLiteral("CAST(substr(year,1,4) AS INTEGER) BETWEEN %1 AND %2"))
                .arg(QString::number(m_dRangeLow),
                        QString::number(m_dRangeHigh));
    }

    return QString();
}
