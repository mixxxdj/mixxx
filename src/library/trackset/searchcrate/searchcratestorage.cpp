#include "library/trackset/searchcrate/searchcratestorage.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackset/searchcrate/searchcrate.h"
#include "library/trackset/searchcrate/searchcrateschema.h"
#include "library/trackset/searchcrate/searchcratesummary.h"
#include "util/db/dbconnection.h"
#include "util/db/fwdsqlquery.h"
#include "util/db/sqllikewildcards.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("SearchCrateStorage");

const QString SEARCHCRATESTABLE_LOCKED = "locked";

const QString SEARCHCRATES_SUMMARY_VIEW = "searchCrate_summary";

const QString SEARCHCRATESSUMMARY_TRACK_COUNT = "track_count";
const QString SEARCHCRATESSUMMARY_TRACK_DURATION = "track_duration";

const QString kSearchCrateTracksJoin =
        QStringLiteral("LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(SEARCHCRATESTABLE,
                        SEARCHCRATESTABLE_ID,
                        SEARCHCRATETRACKSTABLE,
                        SEARCHCRATETRACKSTABLE_SEARCHCRATEID);

const QString kLibraryTracksJoin = kSearchCrateTracksJoin +
        QStringLiteral(" LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(SEARCHCRATETRACKSTABLE,
                        SEARCHCRATETRACKSTABLE_TRACKID,
                        LIBRARY_TABLE,
                        LIBRARYTABLE_ID);

const QString kSearchCrateSummaryViewSelect =
        QStringLiteral(
                "SELECT %1.*,"
                "COUNT(CASE %2.%4 WHEN 0 THEN 1 ELSE NULL END) AS %5,"
                "SUM(CASE %2.%4 WHEN 0 THEN %2.%3 ELSE 0 END) AS %6 "
                "FROM %1")
                .arg(
                        SEARCHCRATESTABLE,
                        LIBRARY_TABLE,
                        LIBRARYTABLE_DURATION,
                        LIBRARYTABLE_MIXXXDELETED,
                        SEARCHCRATESSUMMARY_TRACK_COUNT,
                        SEARCHCRATESSUMMARY_TRACK_DURATION);

const QString kSearchCrateSummaryViewQuery =
        QStringLiteral(
                "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS %2 %3 "
                "GROUP BY %4.%5")
                .arg(
                        SEARCHCRATES_SUMMARY_VIEW,
                        kSearchCrateSummaryViewSelect,
                        kLibraryTracksJoin,
                        SEARCHCRATESTABLE,
                        SEARCHCRATESTABLE_ID);

class SearchCrateQueryBinder final {
  public:
    explicit SearchCrateQueryBinder(FwdSqlQuery& query)
            : m_query(query) {
    }

    void bindId(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, searchCrate.getId());
    }
    void bindName(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, searchCrate.getName());
    }
    void bindLocked(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.isLocked()));
    }
    void bindAutoDjSource(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.isAutoDjSource()));
    }

    void bindSearchInput(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getSearchInput()));
    }
    void bindSearchSql(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getSearchSql()));
    }
    void bindCondition1Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition1Operator()));
    }
    void bindCondition2Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition2Operator()));
    }
    void bindCondition3Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition3Operator()));
    }
    void bindCondition4Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition4Operator()));
    }
    void bindCondition5Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition5Operator()));
    }
    void bindCondition6Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition6Operator()));
    }
    void bindCondition7Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition7Operator()));
    }
    void bindCondition8Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition8Operator()));
    }
    void bindCondition9Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition9Operator()));
    }
    void bindCondition10Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition10Operator()));
    }
    void bindCondition11Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition11Operator()));
    }
    void bindCondition12Operator(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition12Operator()));
    }

    void bindCondition1Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition1Value()));
    }
    void bindCondition2Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition2Value()));
    }
    void bindCondition3Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition3Value()));
    }
    void bindCondition4Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition4Value()));
    }
    void bindCondition5Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition5Value()));
    }
    void bindCondition6Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition6Value()));
    }
    void bindCondition7Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition7Value()));
    }
    void bindCondition8Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition8Value()));
    }
    void bindCondition9Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition9Value()));
    }
    void bindCondition10Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition10Value()));
    }
    void bindCondition11Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition11Value()));
    }
    void bindCondition12Value(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition12Value()));
    }

    void bindCondition1Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition1Combiner()));
    }
    void bindCondition2Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition2Combiner()));
    }
    void bindCondition3Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition3Combiner()));
    }
    void bindCondition4Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition4Combiner()));
    }
    void bindCondition5Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition5Combiner()));
    }
    void bindCondition6Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition6Combiner()));
    }
    void bindCondition7Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition7Combiner()));
    }
    void bindCondition8Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition8Combiner()));
    }
    void bindCondition9Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition9Combiner()));
    }
    void bindCondition10Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition10Combiner()));
    }
    void bindCondition11Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition11Combiner()));
    }
    void bindCondition12Combiner(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition12Combiner()));
    }

    void bindCondition1Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition1Field()));
    }
    void bindCondition2Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition2Field()));
    }
    void bindCondition3Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition3Field()));
    }
    void bindCondition4Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition4Field()));
    }
    void bindCondition5Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition5Field()));
    }
    void bindCondition6Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition6Field()));
    }
    void bindCondition7Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition7Field()));
    }
    void bindCondition8Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition8Field()));
    }
    void bindCondition9Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition9Field()));
    }
    void bindCondition10Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition10Field()));
    }
    void bindCondition11Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition11Field()));
    }
    void bindCondition12Field(const QString& placeholder, const SearchCrate& searchCrate) const {
        m_query.bindValue(placeholder, QVariant(searchCrate.getCondition12Field()));
    }
    void bindConditionField(const QString& fieldName, const SearchCrate& searchCrate, int index) {
        switch (index) {
        case 1:
            bindCondition1Field(fieldName, searchCrate);
            break;
        case 2:
            bindCondition2Field(fieldName, searchCrate);
            break;
        case 3:
            bindCondition3Field(fieldName, searchCrate);
            break;
        case 4:
            bindCondition4Field(fieldName, searchCrate);
            break;
        case 5:
            bindCondition5Field(fieldName, searchCrate);
            break;
        case 6:
            bindCondition6Field(fieldName, searchCrate);
            break;
        case 7:
            bindCondition7Field(fieldName, searchCrate);
            break;
        case 8:
            bindCondition8Field(fieldName, searchCrate);
            break;
        case 9:
            bindCondition9Field(fieldName, searchCrate);
            break;
        case 10:
            bindCondition10Field(fieldName, searchCrate);
            break;
        case 11:
            bindCondition11Field(fieldName, searchCrate);
            break;
        case 12:
            bindCondition12Field(fieldName, searchCrate);
            break;
        default:
            break;
        }
    }

    void bindConditionOperator(const QString& operatorName,
            const SearchCrate& searchCrate,
            int index) {
        switch (index) {
        case 1:
            bindCondition1Operator(operatorName, searchCrate);
            break;
        case 2:
            bindCondition2Operator(operatorName, searchCrate);
            break;
        case 3:
            bindCondition3Operator(operatorName, searchCrate);
            break;
        case 4:
            bindCondition4Operator(operatorName, searchCrate);
            break;
        case 5:
            bindCondition5Operator(operatorName, searchCrate);
            break;
        case 6:
            bindCondition6Operator(operatorName, searchCrate);
            break;
        case 7:
            bindCondition7Operator(operatorName, searchCrate);
            break;
        case 8:
            bindCondition8Operator(operatorName, searchCrate);
            break;
        case 9:
            bindCondition9Operator(operatorName, searchCrate);
            break;
        case 10:
            bindCondition10Operator(operatorName, searchCrate);
            break;
        case 11:
            bindCondition11Operator(operatorName, searchCrate);
            break;
        case 12:
            bindCondition12Operator(operatorName, searchCrate);
            break;
        default:
            break;
        }
    }

    void bindConditionValue(const QString& valueName, const SearchCrate& searchCrate, int index) {
        switch (index) {
        case 1:
            bindCondition1Value(valueName, searchCrate);
            break;
        case 2:
            bindCondition2Value(valueName, searchCrate);
            break;
        case 3:
            bindCondition3Value(valueName, searchCrate);
            break;
        case 4:
            bindCondition4Value(valueName, searchCrate);
            break;
        case 5:
            bindCondition5Value(valueName, searchCrate);
            break;
        case 6:
            bindCondition6Value(valueName, searchCrate);
            break;
        case 7:
            bindCondition7Value(valueName, searchCrate);
            break;
        case 8:
            bindCondition8Value(valueName, searchCrate);
            break;
        case 9:
            bindCondition9Value(valueName, searchCrate);
            break;
        case 10:
            bindCondition10Value(valueName, searchCrate);
            break;
        case 11:
            bindCondition11Value(valueName, searchCrate);
            break;
        case 12:
            bindCondition12Value(valueName, searchCrate);
            break;
        default:
            break;
        }
    }

    void bindConditionCombiner(const QString& combinerName,
            const SearchCrate& searchCrate,
            int index) {
        switch (index) {
        case 1:
            bindCondition1Combiner(combinerName, searchCrate);
            break;
        case 2:
            bindCondition2Combiner(combinerName, searchCrate);
            break;
        case 3:
            bindCondition3Combiner(combinerName, searchCrate);
            break;
        case 4:
            bindCondition4Combiner(combinerName, searchCrate);
            break;
        case 5:
            bindCondition5Combiner(combinerName, searchCrate);
            break;
        case 6:
            bindCondition6Combiner(combinerName, searchCrate);
            break;
        case 7:
            bindCondition7Combiner(combinerName, searchCrate);
            break;
        case 8:
            bindCondition8Combiner(combinerName, searchCrate);
            break;
        case 9:
            bindCondition9Combiner(combinerName, searchCrate);
            break;
        case 10:
            bindCondition10Combiner(combinerName, searchCrate);
            break;
        case 11:
            bindCondition11Combiner(combinerName, searchCrate);
            break;
        case 12:
            bindCondition12Combiner(combinerName, searchCrate);
            break;
        default:
            break;
        }
    }

  protected:
    FwdSqlQuery& m_query;
};

const QChar kSqlListSeparator(',');

// It is not possible to bind multiple values as a list to a query.
// The list of track ids has to be transformed into a single list
// string before it can be used in an SQL query.
QString joinSqlStringList(const QList<TrackId>& trackIds) {
    QString joinedTrackIds;
    // Reserve memory up front to prevent reallocation. Here we
    // assume that all track ids fit into 6 decimal digits and
    // add 1 character for the list separator.
    joinedTrackIds.reserve((6 + 1) * trackIds.size());
    for (const auto& trackId : trackIds) {
        if (!joinedTrackIds.isEmpty()) {
            joinedTrackIds += kSqlListSeparator;
        }
        joinedTrackIds += trackId.toString();
    }
    return joinedTrackIds;
}

} // anonymous namespace

// Next is to make the extra's in the searchCrate name (numbers - duration - locked)
// + duplicate searchCrate
SearchCrateQueryFields::SearchCrateQueryFields(const FwdSqlQuery& query)
        : m_iId(query.fieldIndex(SEARCHCRATESTABLE_ID)),
          m_iName(query.fieldIndex(SEARCHCRATESTABLE_NAME)),
          m_iLocked(query.fieldIndex(SEARCHCRATESTABLE_LOCKED)),
          m_iAutoDjSource(query.fieldIndex(SEARCHCRATESTABLE_AUTODJ_SOURCE)),
          m_iSearchInput(query.fieldIndex(SEARCHCRATESTABLE_SEARCH_INPUT)),
          m_iSearchSql(query.fieldIndex(SEARCHCRATESTABLE_SEARCH_SQL)),
          m_iCondition1Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_1_FIELD)),
          m_iCondition1Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_1_OPERATOR)),
          m_iCondition1Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_1_VALUE)),
          m_iCondition1Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_1_COMBINER)),
          m_iCondition2Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_2_FIELD)),
          m_iCondition2Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_2_OPERATOR)),
          m_iCondition2Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_2_VALUE)),
          m_iCondition2Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_2_COMBINER)),
          m_iCondition3Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_3_FIELD)),
          m_iCondition3Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_3_OPERATOR)),
          m_iCondition3Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_3_VALUE)),
          m_iCondition3Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_3_COMBINER)),
          m_iCondition4Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_4_FIELD)),
          m_iCondition4Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_4_OPERATOR)),
          m_iCondition4Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_4_VALUE)),
          m_iCondition4Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_4_COMBINER)),
          m_iCondition5Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_5_FIELD)),
          m_iCondition5Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_5_OPERATOR)),
          m_iCondition5Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_5_VALUE)),
          m_iCondition5Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_5_COMBINER)),
          m_iCondition6Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_6_FIELD)),
          m_iCondition6Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_6_OPERATOR)),
          m_iCondition6Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_6_VALUE)),
          m_iCondition6Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_6_COMBINER)),
          m_iCondition7Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_7_FIELD)),
          m_iCondition7Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_7_OPERATOR)),
          m_iCondition7Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_7_VALUE)),
          m_iCondition7Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_7_COMBINER)),
          m_iCondition8Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_8_FIELD)),
          m_iCondition8Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_8_OPERATOR)),
          m_iCondition8Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_8_VALUE)),
          m_iCondition8Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_8_COMBINER)),
          m_iCondition9Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_9_FIELD)),
          m_iCondition9Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_9_OPERATOR)),
          m_iCondition9Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_9_VALUE)),
          m_iCondition9Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_9_COMBINER)),
          m_iCondition10Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_10_FIELD)),
          m_iCondition10Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_10_OPERATOR)),
          m_iCondition10Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_10_VALUE)),
          m_iCondition10Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_10_COMBINER)),
          m_iCondition11Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_11_FIELD)),
          m_iCondition11Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_11_OPERATOR)),
          m_iCondition11Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_11_VALUE)),
          m_iCondition11Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_11_COMBINER)),
          m_iCondition12Field(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_12_FIELD)),
          m_iCondition12Operator(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_12_OPERATOR)),
          m_iCondition12Value(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_12_VALUE)),
          m_iCondition12Combiner(query.fieldIndex(SEARCHCRATESTABLE_CONDITION_12_COMBINER)) {
}

// Next is to make the extra's in the searchCrate name (numbers - duration - locked)
// fill fields with values
void SearchCrateQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        SearchCrate* pSearchCrate) const {
    pSearchCrate->setId(getId(query));
    pSearchCrate->setName(getName(query));
    pSearchCrate->setLocked(isLocked(query));
    pSearchCrate->setAutoDjSource(isAutoDjSource(query));
    pSearchCrate->setSearchInput(getSearchInput(query));
    pSearchCrate->setSearchSql(getSearchSql(query));
    pSearchCrate->setCondition1Field(getCondition1Field(query));
    pSearchCrate->setCondition2Field(getCondition2Field(query));
    pSearchCrate->setCondition3Field(getCondition3Field(query));
    pSearchCrate->setCondition4Field(getCondition4Field(query));
    pSearchCrate->setCondition5Field(getCondition5Field(query));
    pSearchCrate->setCondition6Field(getCondition6Field(query));
    pSearchCrate->setCondition7Field(getCondition7Field(query));
    pSearchCrate->setCondition8Field(getCondition8Field(query));
    pSearchCrate->setCondition9Field(getCondition9Field(query));
    pSearchCrate->setCondition10Field(getCondition10Field(query));
    pSearchCrate->setCondition11Field(getCondition11Field(query));
    pSearchCrate->setCondition12Field(getCondition12Field(query));
    pSearchCrate->setCondition1Operator(getCondition1Operator(query));
    pSearchCrate->setCondition2Operator(getCondition2Operator(query));
    pSearchCrate->setCondition3Operator(getCondition3Operator(query));
    pSearchCrate->setCondition4Operator(getCondition4Operator(query));
    pSearchCrate->setCondition5Operator(getCondition5Operator(query));
    pSearchCrate->setCondition6Operator(getCondition6Operator(query));
    pSearchCrate->setCondition7Operator(getCondition7Operator(query));
    pSearchCrate->setCondition8Operator(getCondition8Operator(query));
    pSearchCrate->setCondition9Operator(getCondition9Operator(query));
    pSearchCrate->setCondition10Operator(getCondition10Operator(query));
    pSearchCrate->setCondition11Operator(getCondition11Operator(query));
    pSearchCrate->setCondition12Operator(getCondition12Operator(query));
    pSearchCrate->setCondition1Value(getCondition1Value(query));
    pSearchCrate->setCondition2Value(getCondition2Value(query));
    pSearchCrate->setCondition3Value(getCondition3Value(query));
    pSearchCrate->setCondition4Value(getCondition4Value(query));
    pSearchCrate->setCondition5Value(getCondition5Value(query));
    pSearchCrate->setCondition6Value(getCondition6Value(query));
    pSearchCrate->setCondition7Value(getCondition7Value(query));
    pSearchCrate->setCondition8Value(getCondition8Value(query));
    pSearchCrate->setCondition9Value(getCondition9Value(query));
    pSearchCrate->setCondition10Value(getCondition10Value(query));
    pSearchCrate->setCondition11Value(getCondition11Value(query));
    pSearchCrate->setCondition12Value(getCondition12Value(query));
    pSearchCrate->setCondition1Combiner(getCondition1Combiner(query));
    pSearchCrate->setCondition2Combiner(getCondition2Combiner(query));
    pSearchCrate->setCondition3Combiner(getCondition3Combiner(query));
    pSearchCrate->setCondition4Combiner(getCondition4Combiner(query));
    pSearchCrate->setCondition5Combiner(getCondition5Combiner(query));
    pSearchCrate->setCondition6Combiner(getCondition6Combiner(query));
    pSearchCrate->setCondition7Combiner(getCondition7Combiner(query));
    pSearchCrate->setCondition8Combiner(getCondition8Combiner(query));
    pSearchCrate->setCondition9Combiner(getCondition9Combiner(query));
    pSearchCrate->setCondition10Combiner(getCondition10Combiner(query));
    pSearchCrate->setCondition11Combiner(getCondition11Combiner(query));
    pSearchCrate->setCondition12Combiner(getCondition12Combiner(query));
}

SearchCrateTrackQueryFields::SearchCrateTrackQueryFields(const FwdSqlQuery& query)
        : m_iSearchCrateId(query.fieldIndex(SEARCHCRATETRACKSTABLE_SEARCHCRATEID)),
          m_iTrackId(query.fieldIndex(SEARCHCRATETRACKSTABLE_TRACKID)) {
}

SearchCrateSummaryQueryFields::SearchCrateSummaryQueryFields(const FwdSqlQuery& query)
        : SearchCrateQueryFields(query),
          m_iTrackCount(query.fieldIndex(SEARCHCRATESSUMMARY_TRACK_COUNT)),
          m_iTrackDuration(query.fieldIndex(SEARCHCRATESSUMMARY_TRACK_DURATION)) {
}

void SearchCrateSummaryQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        SearchCrateSummary* pSearchCrateSummary) const {
    SearchCrateQueryFields::populateFromQuery(query, pSearchCrateSummary);
    pSearchCrateSummary->setTrackCount(getTrackCount(query));
    pSearchCrateSummary->setTrackDuration(getTrackDuration(query));
}

void SearchCrateStorage::repairDatabase(const QSqlDatabase& database) {
    // NOTE(uklotzde): No transactions
    // All queries are independent so there is no need to enclose some
    // or all of them in a transaction. Grouping into transactions would
    // improve the overall performance at the cost of increased resource
    // utilization. Since performance is not an issue for a maintenance
    // operation the decision was not to use any transactions.

    // NOTE(uklotzde): Nested scopes
    // Each of the following queries is enclosed in a nested scope.
    // When leaving this scope all resources allocated while executing
    // the query are released implicitly and before executing the next
    // query.

    // SearchCrate
    {
        // Delete searchCrate with empty names
        FwdSqlQuery query(database,
                QStringLiteral("DELETE FROM %1 WHERE %2 IS NULL OR TRIM(%2)=''")
                        .arg(SEARCHCRATESTABLE, SEARCHCRATESTABLE_NAME));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Deleted" << query.numRowsAffected()
                    << "searchCrate with empty names";
        }
    }

    // SearchCrate tracks
    {
        // Remove tracks from non-existent searchCrate
        FwdSqlQuery query(database,
                QStringLiteral(
                        "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)")
                        .arg(SEARCHCRATETRACKSTABLE,
                                SEARCHCRATETRACKSTABLE_SEARCHCRATEID,
                                SEARCHCRATESTABLE_ID,
                                SEARCHCRATESTABLE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning() << "Removed" << query.numRowsAffected()
                              << "searchCrate tracks from non-existent searchCrate";
        }
    }
    {
        // Remove library purged tracks from searchCrate
        FwdSqlQuery query(database,
                QStringLiteral(
                        "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)")
                        .arg(SEARCHCRATETRACKSTABLE,
                                SEARCHCRATETRACKSTABLE_TRACKID,
                                LIBRARYTABLE_ID,
                                LIBRARY_TABLE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning() << "Removed" << query.numRowsAffected()
                              << "library purged tracks from searchCrate";
        }
    }
}

void SearchCrateStorage::connectDatabase(const QSqlDatabase& database) {
    m_database = database;
    createViews();
}

void SearchCrateStorage::disconnectDatabase() {
    // Ensure that we don't use the current database connection
    // any longer.
    m_database = QSqlDatabase();
}

void SearchCrateStorage::createViews() {
    VERIFY_OR_DEBUG_ASSERT(
            FwdSqlQuery(m_database, kSearchCrateSummaryViewQuery).execPrepared()) {
        kLogger.critical()
                << "Failed to create database view for searchCrate summaries!";
    }
}

uint SearchCrateStorage::countSearchCrate() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1").arg(SEARCHCRATESTABLE));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

bool SearchCrateStorage::readSearchCrateById(SearchCrateId id, SearchCrate* pSearchCrate) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(SEARCHCRATESTABLE, SEARCHCRATESTABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        SearchCrateSelectResult searchCrate(std::move(query));
        if ((pSearchCrate != nullptr) ? searchCrate.populateNext(pSearchCrate)
                                      : searchCrate.next()) {
            VERIFY_OR_DEBUG_ASSERT(!searchCrate.next()) {
                kLogger.warning() << "Ambiguous searchCrate id: maybe nul values" << id;
            }
            return true;
        } else {
            kLogger.warning() << "SearchCrate not found by id:" << id;
        }
    }
    return false;
}

bool SearchCrateStorage::readSearchCrateByName(
        const QString& name, SearchCrate* pSearchCrate) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:name")
                    .arg(SEARCHCRATESTABLE, SEARCHCRATESTABLE_NAME));
    query.bindValue(":name", name);
    if (query.execPrepared()) {
        SearchCrateSelectResult searchCrate(std::move(query));
        if ((pSearchCrate != nullptr) ? searchCrate.populateNext(pSearchCrate)
                                      : searchCrate.next()) {
            VERIFY_OR_DEBUG_ASSERT(!searchCrate.next()) {
                kLogger.warning() << "Ambiguous searchCrate name:" << name;
            }
            return true;
        } else {
            if (kLogger.debugEnabled()) {
                kLogger.debug() << "SearchCrate not found by name:" << name;
            }
        }
    }
    return false;
}

SearchCrateSelectResult SearchCrateStorage::selectSearchCrate() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(SEARCHCRATESTABLE, SEARCHCRATESTABLE_NAME)));

    if (query.execPrepared()) {
        return SearchCrateSelectResult(std::move(query));
    } else {
        return SearchCrateSelectResult();
    }
}

SearchCrateSummarySelectResult SearchCrateStorage::selectSearchCrateSummaries() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(SEARCHCRATES_SUMMARY_VIEW, SEARCHCRATESTABLE_NAME)));
    if (query.execPrepared()) {
        return SearchCrateSummarySelectResult(std::move(query));
    } else {
        return SearchCrateSummarySelectResult();
    }
}

bool SearchCrateStorage::readSearchCrateSummaryById(
        SearchCrateId id, SearchCrateSummary* pSearchCrateSummary) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(SEARCHCRATES_SUMMARY_VIEW, SEARCHCRATESTABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        SearchCrateSummarySelectResult searchCrateSummaries(std::move(query));
        if ((pSearchCrateSummary != nullptr)
                        ? searchCrateSummaries.populateNext(pSearchCrateSummary)
                        : searchCrateSummaries.next()) {
            VERIFY_OR_DEBUG_ASSERT(!searchCrateSummaries.next()) {
                kLogger.warning() << "Ambiguous searchCrate id:" << id;
            }
            return true;
        } else {
            kLogger.warning() << "SearchCrate summary not found by id:" << id;
        }
    }
    return false;
}

uint SearchCrateStorage::countSearchCrateTracks(SearchCrateId searchCrateId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2=:searchCrateId")
                    .arg(SEARCHCRATETRACKSTABLE, SEARCHCRATETRACKSTABLE_SEARCHCRATEID));
    query.bindValue(":searchCrateId", searchCrateId);
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

// static
QString SearchCrateStorage::formatSubselectQueryForSearchCrateTrackIds(
        SearchCrateId searchCrateId) {
    return QStringLiteral("SELECT %1 FROM %2 WHERE %3=%4")
            .arg(SEARCHCRATETRACKSTABLE_TRACKID,
                    SEARCHCRATETRACKSTABLE,
                    SEARCHCRATETRACKSTABLE_SEARCHCRATEID,
                    searchCrateId.toString());
}

QString SearchCrateStorage::returnSearchSQLFieldFromTable(SearchCrateId searchCrateId) {
    return QStringLiteral(" %1.%2=%3")
            .arg(SEARCHCRATESTABLE,
                    SEARCHCRATESTABLE_ID,
                    searchCrateId.toString());
}

QString SearchCrateStorage::formatQueryForTrackIdsBySearchCrateNameLike(
        const QString& searchCrateNameLike) const {
    FieldEscaper escaper(m_database);
    QString escapedSearchCrateNameLike = escaper.escapeString(
            kSqlLikeMatchAll + searchCrateNameLike + kSqlLikeMatchAll);
    return QString(
            "SELECT DISTINCT %1 FROM %2 "
            "JOIN %3 ON %4=%5 WHERE %6 LIKE %7 "
            "ORDER BY %1")
            .arg(SEARCHCRATETRACKSTABLE_TRACKID,
                    SEARCHCRATETRACKSTABLE,
                    SEARCHCRATESTABLE,
                    SEARCHCRATETRACKSTABLE_SEARCHCRATEID,
                    SEARCHCRATESTABLE_ID,
                    SEARCHCRATESTABLE_NAME,
                    escapedSearchCrateNameLike);
}

// static
QString SearchCrateStorage::formatQueryForTrackIdsWithSearchCrate() {
    return QStringLiteral(
            "SELECT DISTINCT %1 FROM %2 JOIN %3 ON %4=%5 ORDER BY %1")
            .arg(SEARCHCRATETRACKSTABLE_TRACKID,
                    SEARCHCRATETRACKSTABLE,
                    SEARCHCRATESTABLE,
                    SEARCHCRATETRACKSTABLE_SEARCHCRATEID,
                    SEARCHCRATESTABLE_ID);
}

SearchCrateTrackSelectResult SearchCrateStorage::selectSearchCrateTracksSorted(
        SearchCrateId searchCrateId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:searchCrateId ORDER BY %3")
                    .arg(SEARCHCRATETRACKSTABLE,
                            SEARCHCRATETRACKSTABLE_SEARCHCRATEID,
                            SEARCHCRATETRACKSTABLE_TRACKID));
    query.bindValue(":searchCrateId", searchCrateId);
    if (query.execPrepared()) {
        return SearchCrateTrackSelectResult(std::move(query));
    } else {
        return SearchCrateTrackSelectResult();
    }
}

SearchCrateTrackSelectResult SearchCrateStorage::selectTrackSearchCrateSorted(
        TrackId trackId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:trackId ORDER BY %3")
                    .arg(SEARCHCRATETRACKSTABLE,
                            SEARCHCRATETRACKSTABLE_TRACKID,
                            SEARCHCRATETRACKSTABLE_SEARCHCRATEID));
    query.bindValue(":trackId", trackId);
    if (query.execPrepared()) {
        return SearchCrateTrackSelectResult(std::move(query));
    } else {
        return SearchCrateTrackSelectResult();
    }
}

SearchCrateSummarySelectResult SearchCrateStorage::selectSearchCrateWithTrackCount(
        const QList<TrackId>& trackIds) const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT *, "
                                   "(SELECT COUNT(*) FROM %1 WHERE %2.%3 = %1.%4 and "
                                   "%1.%5 in (%9)) AS %6, "
                                   "0 as %7 FROM %2 ORDER BY %8")
                            .arg(
                                    SEARCHCRATETRACKSTABLE,               // 1
                                    SEARCHCRATESTABLE,                    // 2
                                    SEARCHCRATESTABLE_ID,                 // 3
                                    SEARCHCRATETRACKSTABLE_SEARCHCRATEID, // 4
                                    SEARCHCRATETRACKSTABLE_TRACKID,       // 5
                                    SEARCHCRATESSUMMARY_TRACK_COUNT,      // 6
                                    SEARCHCRATESSUMMARY_TRACK_DURATION,   // 7
                                    SEARCHCRATESTABLE_NAME,               // 8
                                    joinSqlStringList(trackIds))));

    if (query.execPrepared()) {
        return SearchCrateSummarySelectResult(std::move(query));
    } else {
        return SearchCrateSummarySelectResult();
    }
}

SearchCrateTrackSelectResult SearchCrateStorage::selectTracksSortedBySearchCrateNameLike(
        const QString& searchCrateNameLike) const {
    // TODO: Do SQL LIKE wildcards in searchCrateNameLike need to be escaped?
    // Previously we used SqlLikeWildcardEscaper in the past for this
    // purpose. This utility class has become obsolete but could be
    // restored from the 2.3 branch if ever needed again.
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT %1,%2 FROM %3 "
                           "JOIN %4 ON %5 = %6 "
                           "WHERE %7 LIKE :searchCrateNameLike "
                           "ORDER BY %1")
                    .arg(SEARCHCRATETRACKSTABLE_TRACKID,          // 1
                            SEARCHCRATETRACKSTABLE_SEARCHCRATEID, // 2
                            SEARCHCRATETRACKSTABLE,               // 3
                            SEARCHCRATESTABLE,                    // 4
                            SEARCHCRATESTABLE_ID,                 // 5
                            SEARCHCRATETRACKSTABLE_SEARCHCRATEID, // 6
                            SEARCHCRATESTABLE_NAME));             // 7
    query.bindValue(":searchCrateNameLike",
            QVariant(kSqlLikeMatchAll + searchCrateNameLike + kSqlLikeMatchAll));

    if (query.execPrepared()) {
        return SearchCrateTrackSelectResult(std::move(query));
    } else {
        return SearchCrateTrackSelectResult();
    }
}

QSet<SearchCrateId> SearchCrateStorage::collectSearchCrateIdsOfTracks(
        const QList<TrackId>& trackIds) const {
    // NOTE(uklotzde): One query per track id. This could be optimized
    // by querying for chunks of track ids and collecting the results.
    QSet<SearchCrateId> trackSearchCrate;
    for (const auto& trackId : trackIds) {
        // NOTE(uklotzde): The query result does not need to be sorted by searchCrate id
        // here. But since the corresponding FK column is indexed the impact on the
        // performance should be negligible. By reusing an existing query we reduce
        // the amount of code and the number of prepared SQL queries.
        SearchCrateTrackSelectResult searchCrateTracks(selectTrackSearchCrateSorted(trackId));
        while (searchCrateTracks.next()) {
            DEBUG_ASSERT(searchCrateTracks.trackId() == trackId);
            trackSearchCrate.insert(searchCrateTracks.searchCrateId());
        }
    }
    return trackSearchCrate;
}

bool SearchCrateStorage::onInsertingSearchCrate(
        const SearchCrate& searchCrate,
        SearchCrateId* pSearchCrateId) {
    VERIFY_OR_DEBUG_ASSERT(!searchCrate.getId().isValid()) {
        kLogger.warning()
                << "Cannot insert searchCrate with a valid id:" << searchCrate.getId();
        return false;
    }
    FwdSqlQuery query(m_database,
            QStringLiteral("INSERT INTO %1 "
                           "(%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%"
                           "16,%17,%18,%19,%20,%21,%22,%23,%24,%25,%26,%27,%28,"
                           "%29,%30,%31,%32,%33,%34,%35,%36,%37,%38,%39,%40,"
                           "%41,%42,%43,%44,%45,%46,%47,%48,%49,%50,%51,%52,"
                           "%53,%54) "
                           "VALUES (:name, :locked, :autoDjSource, "
                           ":searchInput, :searchSql, "
                           ":condition1_field, :condition1_operator, "
                           ":condition1_value, :condition1_combiner, "
                           ":condition2_field, :condition2_operator, "
                           ":condition2_value, :condition2_combiner, "
                           ":condition3_field, :condition3_operator, "
                           ":condition3_value, :condition3_combiner, "
                           ":condition4_field, :condition4_operator, "
                           ":condition4_value, :condition4_combiner, "
                           ":condition5_field, :condition5_operator, "
                           ":condition5_value, :condition5_combiner, "
                           ":condition6_field, :condition6_operator, "
                           ":condition6_value, :condition6_combiner, "
                           ":condition7_field, :condition7_operator, "
                           ":condition7_value, :condition7_combiner, "
                           ":condition8_field, :condition8_operator, "
                           ":condition8_value, :condition8_combiner, "
                           ":condition9_field, :condition9_operator, "
                           ":condition9_value, :condition9_combiner, "
                           ":condition10_field, :condition10_operator, "
                           ":condition10_value, :condition10_combiner, "
                           ":condition11_field, :condition11_operator, "
                           ":condition11_value, :condition11_combiner, "
                           ":condition12_field, :condition12_operator, "
                           ":condition12_value, :condition12_combiner) ")
                    .arg(SEARCHCRATESTABLE,
                            SEARCHCRATESTABLE_NAME,
                            SEARCHCRATESTABLE_LOCKED,
                            SEARCHCRATESTABLE_AUTODJ_SOURCE,
                            SEARCHCRATESTABLE_SEARCH_INPUT, // 5
                            SEARCHCRATESTABLE_SEARCH_SQL,
                            SEARCHCRATESTABLE_CONDITION_1_FIELD,
                            SEARCHCRATESTABLE_CONDITION_1_OPERATOR,
                            SEARCHCRATESTABLE_CONDITION_1_VALUE,
                            SEARCHCRATESTABLE_CONDITION_1_COMBINER, // 10
                            SEARCHCRATESTABLE_CONDITION_2_FIELD,
                            SEARCHCRATESTABLE_CONDITION_2_OPERATOR,
                            SEARCHCRATESTABLE_CONDITION_2_VALUE,
                            SEARCHCRATESTABLE_CONDITION_2_COMBINER,
                            SEARCHCRATESTABLE_CONDITION_3_FIELD, // 15
                            SEARCHCRATESTABLE_CONDITION_3_OPERATOR,
                            SEARCHCRATESTABLE_CONDITION_3_VALUE,
                            SEARCHCRATESTABLE_CONDITION_3_COMBINER,
                            SEARCHCRATESTABLE_CONDITION_4_FIELD,
                            SEARCHCRATESTABLE_CONDITION_4_OPERATOR, // 20
                            SEARCHCRATESTABLE_CONDITION_4_VALUE,
                            SEARCHCRATESTABLE_CONDITION_4_COMBINER,
                            SEARCHCRATESTABLE_CONDITION_5_FIELD,
                            SEARCHCRATESTABLE_CONDITION_5_OPERATOR,
                            SEARCHCRATESTABLE_CONDITION_5_VALUE, // 25
                            SEARCHCRATESTABLE_CONDITION_5_COMBINER,
                            SEARCHCRATESTABLE_CONDITION_6_FIELD,
                            SEARCHCRATESTABLE_CONDITION_6_OPERATOR,
                            SEARCHCRATESTABLE_CONDITION_6_VALUE,
                            SEARCHCRATESTABLE_CONDITION_6_COMBINER, // 30
                            SEARCHCRATESTABLE_CONDITION_7_FIELD,
                            SEARCHCRATESTABLE_CONDITION_7_OPERATOR,
                            SEARCHCRATESTABLE_CONDITION_7_VALUE,
                            SEARCHCRATESTABLE_CONDITION_7_COMBINER,
                            SEARCHCRATESTABLE_CONDITION_8_FIELD, // 35
                            SEARCHCRATESTABLE_CONDITION_8_OPERATOR,
                            SEARCHCRATESTABLE_CONDITION_8_VALUE,
                            SEARCHCRATESTABLE_CONDITION_8_COMBINER,
                            SEARCHCRATESTABLE_CONDITION_9_FIELD,
                            SEARCHCRATESTABLE_CONDITION_9_OPERATOR, // 40
                            SEARCHCRATESTABLE_CONDITION_9_VALUE,
                            SEARCHCRATESTABLE_CONDITION_9_COMBINER,
                            SEARCHCRATESTABLE_CONDITION_10_FIELD,
                            SEARCHCRATESTABLE_CONDITION_10_OPERATOR,
                            SEARCHCRATESTABLE_CONDITION_10_VALUE, // 45
                            SEARCHCRATESTABLE_CONDITION_10_COMBINER,
                            SEARCHCRATESTABLE_CONDITION_11_FIELD,
                            SEARCHCRATESTABLE_CONDITION_11_OPERATOR,
                            SEARCHCRATESTABLE_CONDITION_11_VALUE,
                            SEARCHCRATESTABLE_CONDITION_11_COMBINER, // 50
                            SEARCHCRATESTABLE_CONDITION_12_FIELD,
                            SEARCHCRATESTABLE_CONDITION_12_OPERATOR,
                            SEARCHCRATESTABLE_CONDITION_12_VALUE,
                            SEARCHCRATESTABLE_CONDITION_12_COMBINER)); // 54

    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    SearchCrateQueryBinder queryBinder(query);
    queryBinder.bindName(":name", searchCrate);
    queryBinder.bindLocked(":locked", searchCrate);
    queryBinder.bindAutoDjSource(":autoDjSource", searchCrate);
    queryBinder.bindSearchInput(":searchInput", searchCrate);
    queryBinder.bindSearchSql(":searchSql", searchCrate);
    // Loop from 1 to 12 to bind all condition parameters dynamically
    // Call the corresponding bind methods on queryBinder, querybinder procedure
    // is longer but this procedure is a bit smaller... I don't know what's
    // right
    for (int i = 1; i <= 12; ++i) {
        QString suffix = QString::number(i);
        queryBinder.bindConditionField(QString(":condition%1_field").arg(suffix), searchCrate, i);
        queryBinder.bindConditionOperator(
                QString(":condition%1_operator").arg(suffix), searchCrate, i);
        queryBinder.bindConditionValue(QString(":condition%1_value").arg(suffix), searchCrate, i);
        queryBinder.bindConditionCombiner(
                QString(":condition%1_combiner").arg(suffix), searchCrate, i);
    }

    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }

    if (query.numRowsAffected() > 0) {
        DEBUG_ASSERT(query.numRowsAffected() == 1);
        if (pSearchCrateId != nullptr) {
            *pSearchCrateId = SearchCrateId(query.lastInsertId());
            DEBUG_ASSERT(pSearchCrateId->isValid());
        }
        return true;
    } else {
        return false;
    }
}

bool SearchCrateStorage::onUpdatingSearchCrate(
        const SearchCrate& searchCrate) {
    VERIFY_OR_DEBUG_ASSERT(searchCrate.getId().isValid()) {
        kLogger.warning()
                << "Cannot update searchCrate without a valid id";
        return false;
    }
    FwdSqlQuery query(m_database,
            QString(
                    "UPDATE %1 "
                    "SET %2=:name,%3=:locked,%4=:autoDjSource, %5=:searchInput, %6=:searchSql "
                    "WHERE %7=:id")
                    .arg(
                            SEARCHCRATESTABLE,
                            SEARCHCRATESTABLE_NAME,
                            SEARCHCRATESTABLE_LOCKED,
                            SEARCHCRATESTABLE_AUTODJ_SOURCE,
                            SEARCHCRATESTABLE_SEARCH_INPUT,
                            SEARCHCRATESTABLE_SEARCH_SQL,
                            SEARCHCRATESTABLE_ID));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    SearchCrateQueryBinder queryBinder(query);
    queryBinder.bindId(":id", searchCrate);
    queryBinder.bindName(":name", searchCrate);
    queryBinder.bindLocked(":locked", searchCrate);
    queryBinder.bindAutoDjSource(":autoDjSource", searchCrate);
    queryBinder.bindSearchInput(":searchInput", searchCrate);
    queryBinder.bindSearchSql(":searchSql", searchCrate);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    if (query.numRowsAffected() > 0) {
        VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
            kLogger.warning()
                    << "Updated multiple searchCrate with the same id" << searchCrate.getId();
        }
        return true;
    } else {
        kLogger.warning()
                << "Cannot update non-existent searchCrate with id" << searchCrate.getId();
        return false;
    }
}

bool SearchCrateStorage::onDeletingSearchCrate(
        SearchCrateId searchCrateId) {
    VERIFY_OR_DEBUG_ASSERT(searchCrateId.isValid()) {
        kLogger.warning()
                << "Cannot delete searchCrate without a valid id";
        return false;
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(SEARCHCRATETRACKSTABLE, SEARCHCRATETRACKSTABLE_SEARCHCRATEID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", searchCrateId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() <= 0) {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Deleting empty searchCrate with id"
                        << searchCrateId;
            }
        }
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(SEARCHCRATESTABLE, SEARCHCRATESTABLE_ID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", searchCrateId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() > 0) {
            VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
                kLogger.warning()
                        << "Deleted multiple searchCrate with the same id" << searchCrateId;
            }
            return true;
        } else {
            kLogger.warning()
                    << "Cannot delete non-existent searchCrate with id" << searchCrateId;
            return false;
        }
    }
}

bool SearchCrateStorage::onAddingSearchCrateTracks(
        SearchCrateId searchCrateId,
        const QList<TrackId>& trackIds) {
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "INSERT OR IGNORE INTO %1 (%2, %3) "
                    "VALUES (:searchCrateId,:trackId)")
                    .arg(
                            SEARCHCRATETRACKSTABLE,
                            SEARCHCRATETRACKSTABLE_SEARCHCRATEID,
                            SEARCHCRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":searchCrateId", searchCrateId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track is already in searchCrate
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not added to searchCrate" << searchCrateId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool SearchCrateStorage::onRemovingSearchCrateTracks(
        SearchCrateId searchCrateId,
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): We remove tracks in a loop
    // analogously to adding tracks (see above).
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "DELETE FROM %1 "
                    "WHERE %2=:searchCrateId AND %3=:trackId")
                    .arg(
                            SEARCHCRATETRACKSTABLE,
                            SEARCHCRATETRACKSTABLE_SEARCHCRATEID,
                            SEARCHCRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":searchCrateId", searchCrateId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track not found in searchCrate
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not removed from searchCrate" << searchCrateId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool SearchCrateStorage::onPurgingTracks(
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): Remove tracks from searchCrate one-by-one.
    // This might be optimized by deleting multiple track ids
    // at once in chunks with a maximum size.
    FwdSqlQuery query(m_database,
            QStringLiteral("DELETE FROM %1 WHERE %2=:trackId")
                    .arg(SEARCHCRATETRACKSTABLE, SEARCHCRATETRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
    }
    return true;
}
