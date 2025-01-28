#include "library/trackset/smarties/smartiesstorage.h"

#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackset/smarties/smarties.h"
#include "library/trackset/smarties/smartiesschema.h"
#include "library/trackset/smarties/smartiessummary.h"
#include "util/db/dbconnection.h"
#include "util/db/fwdsqlquery.h"
#include "util/db/sqllikewildcards.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("SmartiesStorage");

const QString SMARTIESTABLE_LOCKED = "locked";

const QString SMARTIES_SUMMARY_VIEW = "smarties_summary";

const QString SMARTIESSUMMARY_TRACK_COUNT = "track_count";
const QString SMARTIESSUMMARY_TRACK_DURATION = "track_duration";

const QString kSmartiesTracksJoin =
        QStringLiteral("LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(SMARTIES_TABLE,
                        SMARTIESTABLE_ID,
                        SMARTIES_TRACKS_TABLE,
                        SMARTIESTRACKSTABLE_SMARTIESID);

const QString kLibraryTracksJoin = kSmartiesTracksJoin +
        QStringLiteral(" LEFT JOIN %3 ON %3.%4=%1.%2")
                .arg(SMARTIES_TRACKS_TABLE,
                        SMARTIESTRACKSTABLE_TRACKID,
                        LIBRARY_TABLE,
                        LIBRARYTABLE_ID);

const QString kSmartiesSummaryViewSelect =
        QStringLiteral(
                "SELECT %1.*,"
                "COUNT(CASE %2.%4 WHEN 0 THEN 1 ELSE NULL END) AS %5,"
                "SUM(CASE %2.%4 WHEN 0 THEN %2.%3 ELSE 0 END) AS %6 "
                "FROM %1")
                .arg(
                        SMARTIES_TABLE,
                        LIBRARY_TABLE,
                        LIBRARYTABLE_DURATION,
                        LIBRARYTABLE_MIXXXDELETED,
                        SMARTIESSUMMARY_TRACK_COUNT,
                        SMARTIESSUMMARY_TRACK_DURATION);

const QString kSmartiesSummaryViewQuery =
        QStringLiteral(
                "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS %2 %3 "
                "GROUP BY %4.%5")
                .arg(
                        SMARTIES_SUMMARY_VIEW,
                        kSmartiesSummaryViewSelect,
                        kLibraryTracksJoin,
                        SMARTIES_TABLE,
                        SMARTIESTABLE_ID);

class SmartiesQueryBinder final {
  public:
    explicit SmartiesQueryBinder(FwdSqlQuery& query)
            : m_query(query) {
    }

    void bindId(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, smarties.getId());
    }
    void bindName(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, smarties.getName());
    }
    void bindLocked(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.isLocked()));
    }
    void bindAutoDjSource(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.isAutoDjSource()));
    }

    void bindSearchInput(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getSearchInput()));
    }
    void bindSearchSql(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getSearchSql()));
    }
    void bindCondition1Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition1Operator()));
    }
    void bindCondition2Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition2Operator()));
    }
    void bindCondition3Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition3Operator()));
    }
    void bindCondition4Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition4Operator()));
    }
    void bindCondition5Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition5Operator()));
    }
    void bindCondition6Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition6Operator()));
    }
    void bindCondition7Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition7Operator()));
    }
    void bindCondition8Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition8Operator()));
    }
    void bindCondition9Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition9Operator()));
    }
    void bindCondition10Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition10Operator()));
    }
    void bindCondition11Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition11Operator()));
    }
    void bindCondition12Operator(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition12Operator()));
    }

    void bindCondition1Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition1Value()));
    }
    void bindCondition2Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition2Value()));
    }
    void bindCondition3Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition3Value()));
    }
    void bindCondition4Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition4Value()));
    }
    void bindCondition5Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition5Value()));
    }
    void bindCondition6Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition6Value()));
    }
    void bindCondition7Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition7Value()));
    }
    void bindCondition8Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition8Value()));
    }
    void bindCondition9Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition9Value()));
    }
    void bindCondition10Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition10Value()));
    }
    void bindCondition11Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition11Value()));
    }
    void bindCondition12Value(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition12Value()));
    }

    void bindCondition1Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition1Combiner()));
    }
    void bindCondition2Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition2Combiner()));
    }
    void bindCondition3Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition3Combiner()));
    }
    void bindCondition4Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition4Combiner()));
    }
    void bindCondition5Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition5Combiner()));
    }
    void bindCondition6Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition6Combiner()));
    }
    void bindCondition7Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition7Combiner()));
    }
    void bindCondition8Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition8Combiner()));
    }
    void bindCondition9Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition9Combiner()));
    }
    void bindCondition10Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition10Combiner()));
    }
    void bindCondition11Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition11Combiner()));
    }
    void bindCondition12Combiner(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition12Combiner()));
    }

    void bindCondition1Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition1Field()));
    }
    void bindCondition2Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition2Field()));
    }
    void bindCondition3Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition3Field()));
    }
    void bindCondition4Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition4Field()));
    }
    void bindCondition5Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition5Field()));
    }
    void bindCondition6Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition6Field()));
    }
    void bindCondition7Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition7Field()));
    }
    void bindCondition8Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition8Field()));
    }
    void bindCondition9Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition9Field()));
    }
    void bindCondition10Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition10Field()));
    }
    void bindCondition11Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition11Field()));
    }
    void bindCondition12Field(const QString& placeholder, const Smarties& smarties) const {
        m_query.bindValue(placeholder, QVariant(smarties.getCondition12Field()));
    }
    void bindConditionField(const QString& fieldName, const Smarties& smarties, int index) {
        switch (index) {
        case 1:
            bindCondition1Field(fieldName, smarties);
            break;
        case 2:
            bindCondition2Field(fieldName, smarties);
            break;
        case 3:
            bindCondition3Field(fieldName, smarties);
            break;
        case 4:
            bindCondition4Field(fieldName, smarties);
            break;
        case 5:
            bindCondition5Field(fieldName, smarties);
            break;
        case 6:
            bindCondition6Field(fieldName, smarties);
            break;
        case 7:
            bindCondition7Field(fieldName, smarties);
            break;
        case 8:
            bindCondition8Field(fieldName, smarties);
            break;
        case 9:
            bindCondition9Field(fieldName, smarties);
            break;
        case 10:
            bindCondition10Field(fieldName, smarties);
            break;
        case 11:
            bindCondition11Field(fieldName, smarties);
            break;
        case 12:
            bindCondition12Field(fieldName, smarties);
            break;
        default:
            break;
        }
    }

    void bindConditionOperator(const QString& operatorName, const Smarties& smarties, int index) {
        switch (index) {
        case 1:
            bindCondition1Operator(operatorName, smarties);
            break;
        case 2:
            bindCondition2Operator(operatorName, smarties);
            break;
        case 3:
            bindCondition3Operator(operatorName, smarties);
            break;
        case 4:
            bindCondition4Operator(operatorName, smarties);
            break;
        case 5:
            bindCondition5Operator(operatorName, smarties);
            break;
        case 6:
            bindCondition6Operator(operatorName, smarties);
            break;
        case 7:
            bindCondition7Operator(operatorName, smarties);
            break;
        case 8:
            bindCondition8Operator(operatorName, smarties);
            break;
        case 9:
            bindCondition9Operator(operatorName, smarties);
            break;
        case 10:
            bindCondition10Operator(operatorName, smarties);
            break;
        case 11:
            bindCondition11Operator(operatorName, smarties);
            break;
        case 12:
            bindCondition12Operator(operatorName, smarties);
            break;
        default:
            break;
        }
    }

    void bindConditionValue(const QString& valueName, const Smarties& smarties, int index) {
        switch (index) {
        case 1:
            bindCondition1Value(valueName, smarties);
            break;
        case 2:
            bindCondition2Value(valueName, smarties);
            break;
        case 3:
            bindCondition3Value(valueName, smarties);
            break;
        case 4:
            bindCondition4Value(valueName, smarties);
            break;
        case 5:
            bindCondition5Value(valueName, smarties);
            break;
        case 6:
            bindCondition6Value(valueName, smarties);
            break;
        case 7:
            bindCondition7Value(valueName, smarties);
            break;
        case 8:
            bindCondition8Value(valueName, smarties);
            break;
        case 9:
            bindCondition9Value(valueName, smarties);
            break;
        case 10:
            bindCondition10Value(valueName, smarties);
            break;
        case 11:
            bindCondition11Value(valueName, smarties);
            break;
        case 12:
            bindCondition12Value(valueName, smarties);
            break;
        default:
            break;
        }
    }

    void bindConditionCombiner(const QString& combinerName, const Smarties& smarties, int index) {
        switch (index) {
        case 1:
            bindCondition1Combiner(combinerName, smarties);
            break;
        case 2:
            bindCondition2Combiner(combinerName, smarties);
            break;
        case 3:
            bindCondition3Combiner(combinerName, smarties);
            break;
        case 4:
            bindCondition4Combiner(combinerName, smarties);
            break;
        case 5:
            bindCondition5Combiner(combinerName, smarties);
            break;
        case 6:
            bindCondition6Combiner(combinerName, smarties);
            break;
        case 7:
            bindCondition7Combiner(combinerName, smarties);
            break;
        case 8:
            bindCondition8Combiner(combinerName, smarties);
            break;
        case 9:
            bindCondition9Combiner(combinerName, smarties);
            break;
        case 10:
            bindCondition10Combiner(combinerName, smarties);
            break;
        case 11:
            bindCondition11Combiner(combinerName, smarties);
            break;
        case 12:
            bindCondition12Combiner(combinerName, smarties);
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

// Next is to make the extra's in the smarties name (numbers - duration - locked)
// + duplicate smarties
SmartiesQueryFields::SmartiesQueryFields(const FwdSqlQuery& query)
        : m_iId(query.fieldIndex(SMARTIESTABLE_ID)),
          m_iName(query.fieldIndex(SMARTIESTABLE_NAME)),
          m_iLocked(query.fieldIndex(SMARTIESTABLE_LOCKED)),
          m_iAutoDjSource(query.fieldIndex(SMARTIESTABLE_AUTODJ_SOURCE)),
          m_iSearchInput(query.fieldIndex(SMARTIESTABLE_SEARCH_INPUT)),
          m_iSearchSql(query.fieldIndex(SMARTIESTABLE_SEARCH_SQL)),
          m_iCondition1Field(query.fieldIndex(SMARTIESTABLE_CONDITION_1_FIELD)),
          m_iCondition1Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_1_OPERATOR)),
          m_iCondition1Value(query.fieldIndex(SMARTIESTABLE_CONDITION_1_VALUE)),
          m_iCondition1Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_1_COMBINER)),
          m_iCondition2Field(query.fieldIndex(SMARTIESTABLE_CONDITION_2_FIELD)),
          m_iCondition2Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_2_OPERATOR)),
          m_iCondition2Value(query.fieldIndex(SMARTIESTABLE_CONDITION_2_VALUE)),
          m_iCondition2Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_2_COMBINER)),
          m_iCondition3Field(query.fieldIndex(SMARTIESTABLE_CONDITION_3_FIELD)),
          m_iCondition3Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_3_OPERATOR)),
          m_iCondition3Value(query.fieldIndex(SMARTIESTABLE_CONDITION_3_VALUE)),
          m_iCondition3Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_3_COMBINER)),
          m_iCondition4Field(query.fieldIndex(SMARTIESTABLE_CONDITION_4_FIELD)),
          m_iCondition4Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_4_OPERATOR)),
          m_iCondition4Value(query.fieldIndex(SMARTIESTABLE_CONDITION_4_VALUE)),
          m_iCondition4Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_4_COMBINER)),
          m_iCondition5Field(query.fieldIndex(SMARTIESTABLE_CONDITION_5_FIELD)),
          m_iCondition5Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_5_OPERATOR)),
          m_iCondition5Value(query.fieldIndex(SMARTIESTABLE_CONDITION_5_VALUE)),
          m_iCondition5Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_5_COMBINER)),
          m_iCondition6Field(query.fieldIndex(SMARTIESTABLE_CONDITION_6_FIELD)),
          m_iCondition6Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_6_OPERATOR)),
          m_iCondition6Value(query.fieldIndex(SMARTIESTABLE_CONDITION_6_VALUE)),
          m_iCondition6Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_6_COMBINER)),
          m_iCondition7Field(query.fieldIndex(SMARTIESTABLE_CONDITION_7_FIELD)),
          m_iCondition7Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_7_OPERATOR)),
          m_iCondition7Value(query.fieldIndex(SMARTIESTABLE_CONDITION_7_VALUE)),
          m_iCondition7Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_7_COMBINER)),
          m_iCondition8Field(query.fieldIndex(SMARTIESTABLE_CONDITION_8_FIELD)),
          m_iCondition8Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_8_OPERATOR)),
          m_iCondition8Value(query.fieldIndex(SMARTIESTABLE_CONDITION_8_VALUE)),
          m_iCondition8Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_8_COMBINER)),
          m_iCondition9Field(query.fieldIndex(SMARTIESTABLE_CONDITION_9_FIELD)),
          m_iCondition9Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_9_OPERATOR)),
          m_iCondition9Value(query.fieldIndex(SMARTIESTABLE_CONDITION_9_VALUE)),
          m_iCondition9Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_9_COMBINER)),
          m_iCondition10Field(query.fieldIndex(SMARTIESTABLE_CONDITION_10_FIELD)),
          m_iCondition10Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_10_OPERATOR)),
          m_iCondition10Value(query.fieldIndex(SMARTIESTABLE_CONDITION_10_VALUE)),
          m_iCondition10Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_10_COMBINER)),
          m_iCondition11Field(query.fieldIndex(SMARTIESTABLE_CONDITION_11_FIELD)),
          m_iCondition11Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_11_OPERATOR)),
          m_iCondition11Value(query.fieldIndex(SMARTIESTABLE_CONDITION_11_VALUE)),
          m_iCondition11Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_11_COMBINER)),
          m_iCondition12Field(query.fieldIndex(SMARTIESTABLE_CONDITION_12_FIELD)),
          m_iCondition12Operator(query.fieldIndex(SMARTIESTABLE_CONDITION_12_OPERATOR)),
          m_iCondition12Value(query.fieldIndex(SMARTIESTABLE_CONDITION_12_VALUE)),
          m_iCondition12Combiner(query.fieldIndex(SMARTIESTABLE_CONDITION_12_COMBINER)) {
}

// Next is to make the extra's in the smarties name (numbers - duration - locked)
// fill fields with values
void SmartiesQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        Smarties* pSmarties) const {
    pSmarties->setId(getId(query));
    pSmarties->setName(getName(query));
    pSmarties->setLocked(isLocked(query));
    pSmarties->setAutoDjSource(isAutoDjSource(query));
    pSmarties->setSearchInput(getSearchInput(query));
    pSmarties->setSearchSql(getSearchSql(query));
    pSmarties->setCondition1Field(getCondition1Field(query));
    pSmarties->setCondition2Field(getCondition2Field(query));
    pSmarties->setCondition3Field(getCondition3Field(query));
    pSmarties->setCondition4Field(getCondition4Field(query));
    pSmarties->setCondition5Field(getCondition5Field(query));
    pSmarties->setCondition6Field(getCondition6Field(query));
    pSmarties->setCondition7Field(getCondition7Field(query));
    pSmarties->setCondition8Field(getCondition8Field(query));
    pSmarties->setCondition9Field(getCondition9Field(query));
    pSmarties->setCondition10Field(getCondition10Field(query));
    pSmarties->setCondition11Field(getCondition11Field(query));
    pSmarties->setCondition12Field(getCondition12Field(query));
    pSmarties->setCondition1Operator(getCondition1Operator(query));
    pSmarties->setCondition2Operator(getCondition2Operator(query));
    pSmarties->setCondition3Operator(getCondition3Operator(query));
    pSmarties->setCondition4Operator(getCondition4Operator(query));
    pSmarties->setCondition5Operator(getCondition5Operator(query));
    pSmarties->setCondition6Operator(getCondition6Operator(query));
    pSmarties->setCondition7Operator(getCondition7Operator(query));
    pSmarties->setCondition8Operator(getCondition8Operator(query));
    pSmarties->setCondition9Operator(getCondition9Operator(query));
    pSmarties->setCondition10Operator(getCondition10Operator(query));
    pSmarties->setCondition11Operator(getCondition11Operator(query));
    pSmarties->setCondition12Operator(getCondition12Operator(query));
    pSmarties->setCondition1Value(getCondition1Value(query));
    pSmarties->setCondition2Value(getCondition2Value(query));
    pSmarties->setCondition3Value(getCondition3Value(query));
    pSmarties->setCondition4Value(getCondition4Value(query));
    pSmarties->setCondition5Value(getCondition5Value(query));
    pSmarties->setCondition6Value(getCondition6Value(query));
    pSmarties->setCondition7Value(getCondition7Value(query));
    pSmarties->setCondition8Value(getCondition8Value(query));
    pSmarties->setCondition9Value(getCondition9Value(query));
    pSmarties->setCondition10Value(getCondition10Value(query));
    pSmarties->setCondition11Value(getCondition11Value(query));
    pSmarties->setCondition12Value(getCondition12Value(query));
    pSmarties->setCondition1Combiner(getCondition1Combiner(query));
    pSmarties->setCondition2Combiner(getCondition2Combiner(query));
    pSmarties->setCondition3Combiner(getCondition3Combiner(query));
    pSmarties->setCondition4Combiner(getCondition4Combiner(query));
    pSmarties->setCondition5Combiner(getCondition5Combiner(query));
    pSmarties->setCondition6Combiner(getCondition6Combiner(query));
    pSmarties->setCondition7Combiner(getCondition7Combiner(query));
    pSmarties->setCondition8Combiner(getCondition8Combiner(query));
    pSmarties->setCondition9Combiner(getCondition9Combiner(query));
    pSmarties->setCondition10Combiner(getCondition10Combiner(query));
    pSmarties->setCondition11Combiner(getCondition11Combiner(query));
    pSmarties->setCondition12Combiner(getCondition12Combiner(query));
}

SmartiesTrackQueryFields::SmartiesTrackQueryFields(const FwdSqlQuery& query)
        : m_iSmartiesId(query.fieldIndex(SMARTIESTRACKSTABLE_SMARTIESID)),
          m_iTrackId(query.fieldIndex(SMARTIESTRACKSTABLE_TRACKID)) {
}

SmartiesSummaryQueryFields::SmartiesSummaryQueryFields(const FwdSqlQuery& query)
        : SmartiesQueryFields(query),
          m_iTrackCount(query.fieldIndex(SMARTIESSUMMARY_TRACK_COUNT)),
          m_iTrackDuration(query.fieldIndex(SMARTIESSUMMARY_TRACK_DURATION)) {
}

void SmartiesSummaryQueryFields::populateFromQuery(
        const FwdSqlQuery& query,
        SmartiesSummary* pSmartiesSummary) const {
    SmartiesQueryFields::populateFromQuery(query, pSmartiesSummary);
    pSmartiesSummary->setTrackCount(getTrackCount(query));
    pSmartiesSummary->setTrackDuration(getTrackDuration(query));
}

void SmartiesStorage::repairDatabase(const QSqlDatabase& database) {
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

    // Smarties
    {
        // Delete smarties with empty names
        FwdSqlQuery query(database,
                QStringLiteral("DELETE FROM %1 WHERE %2 IS NULL OR TRIM(%2)=''")
                        .arg(SMARTIES_TABLE, SMARTIESTABLE_NAME));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning()
                    << "Deleted" << query.numRowsAffected()
                    << "smarties with empty names";
        }
    }

    // Smarties tracks
    {
        // Remove tracks from non-existent smarties
        FwdSqlQuery query(database,
                QStringLiteral(
                        "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)")
                        .arg(SMARTIES_TRACKS_TABLE,
                                SMARTIESTRACKSTABLE_SMARTIESID,
                                SMARTIESTABLE_ID,
                                SMARTIES_TABLE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning() << "Removed" << query.numRowsAffected()
                              << "smarties tracks from non-existent smarties";
        }
    }
    {
        // Remove library purged tracks from smarties
        FwdSqlQuery query(database,
                QStringLiteral(
                        "DELETE FROM %1 WHERE %2 NOT IN (SELECT %3 FROM %4)")
                        .arg(SMARTIES_TRACKS_TABLE,
                                SMARTIESTRACKSTABLE_TRACKID,
                                LIBRARYTABLE_ID,
                                LIBRARY_TABLE));
        if (query.execPrepared() && (query.numRowsAffected() > 0)) {
            kLogger.warning() << "Removed" << query.numRowsAffected()
                              << "library purged tracks from smarties";
        }
    }
}

void SmartiesStorage::connectDatabase(const QSqlDatabase& database) {
    m_database = database;
    createViews();
}

void SmartiesStorage::disconnectDatabase() {
    // Ensure that we don't use the current database connection
    // any longer.
    m_database = QSqlDatabase();
}

void SmartiesStorage::createViews() {
    VERIFY_OR_DEBUG_ASSERT(
            FwdSqlQuery(m_database, kSmartiesSummaryViewQuery).execPrepared()) {
        kLogger.critical()
                << "Failed to create database view for smarties summaries!";
    }
}

uint SmartiesStorage::countSmarties() const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1").arg(SMARTIES_TABLE));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

bool SmartiesStorage::readSmartiesById(SmartiesId id, Smarties* pSmarties) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(SMARTIES_TABLE, SMARTIESTABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        SmartiesSelectResult smarties(std::move(query));
        if ((pSmarties != nullptr) ? smarties.populateNext(pSmarties) : smarties.next()) {
            VERIFY_OR_DEBUG_ASSERT(!smarties.next()) {
                kLogger.warning() << "Ambiguous smarties id: maybe nul values" << id;
            }
            return true;
        } else {
            kLogger.warning() << "Smarties not found by id:" << id;
        }
    }
    return false;
}

bool SmartiesStorage::readSmartiesByName(const QString& name, Smarties* pSmarties) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:name")
                    .arg(SMARTIES_TABLE, SMARTIESTABLE_NAME));
    query.bindValue(":name", name);
    if (query.execPrepared()) {
        SmartiesSelectResult smarties(std::move(query));
        if ((pSmarties != nullptr) ? smarties.populateNext(pSmarties) : smarties.next()) {
            VERIFY_OR_DEBUG_ASSERT(!smarties.next()) {
                kLogger.warning() << "Ambiguous smarties name:" << name;
            }
            return true;
        } else {
            if (kLogger.debugEnabled()) {
                kLogger.debug() << "Smarties not found by name:" << name;
            }
        }
    }
    return false;
}

SmartiesSelectResult SmartiesStorage::selectSmarties() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(SMARTIES_TABLE, SMARTIESTABLE_NAME)));

    if (query.execPrepared()) {
        return SmartiesSelectResult(std::move(query));
    } else {
        return SmartiesSelectResult();
    }
}

SmartiesSummarySelectResult SmartiesStorage::selectSmartiesSummaries() const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT * FROM %1 ORDER BY %2")
                            .arg(SMARTIES_SUMMARY_VIEW, SMARTIESTABLE_NAME)));
    if (query.execPrepared()) {
        return SmartiesSummarySelectResult(std::move(query));
    } else {
        return SmartiesSummarySelectResult();
    }
}

bool SmartiesStorage::readSmartiesSummaryById(
        SmartiesId id, SmartiesSummary* pSmartiesSummary) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:id")
                    .arg(SMARTIES_SUMMARY_VIEW, SMARTIESTABLE_ID));
    query.bindValue(":id", id);
    if (query.execPrepared()) {
        SmartiesSummarySelectResult smartiesSummaries(std::move(query));
        if ((pSmartiesSummary != nullptr)
                        ? smartiesSummaries.populateNext(pSmartiesSummary)
                        : smartiesSummaries.next()) {
            VERIFY_OR_DEBUG_ASSERT(!smartiesSummaries.next()) {
                kLogger.warning() << "Ambiguous smarties id:" << id;
            }
            return true;
        } else {
            kLogger.warning() << "Smarties summary not found by id:" << id;
        }
    }
    return false;
}

uint SmartiesStorage::countSmartiesTracks(SmartiesId smartiesId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2=:smartiesId")
                    .arg(SMARTIES_TRACKS_TABLE, SMARTIESTRACKSTABLE_SMARTIESID));
    query.bindValue(":smartiesId", smartiesId);
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

// static
QString SmartiesStorage::formatSubselectQueryForSmartiesTrackIds(SmartiesId smartiesId) {
    return QStringLiteral("SELECT %1 FROM %2 WHERE %3=%4")
            .arg(SMARTIESTRACKSTABLE_TRACKID,
                    SMARTIES_TRACKS_TABLE,
                    SMARTIESTRACKSTABLE_SMARTIESID,
                    smartiesId.toString());
}

QString SmartiesStorage::returnSearchSQLFieldFromTable(SmartiesId smartiesId) {
    return QStringLiteral(" %1.%2=%3")
            .arg(SMARTIES_TABLE,
                    SMARTIESTABLE_ID,
                    smartiesId.toString());
}

QString SmartiesStorage::formatQueryForTrackIdsBySmartiesNameLike(
        const QString& smartiesNameLike) const {
    FieldEscaper escaper(m_database);
    QString escapedSmartiesNameLike = escaper.escapeString(
            kSqlLikeMatchAll + smartiesNameLike + kSqlLikeMatchAll);
    return QString(
            "SELECT DISTINCT %1 FROM %2 "
            "JOIN %3 ON %4=%5 WHERE %6 LIKE %7 "
            "ORDER BY %1")
            .arg(SMARTIESTRACKSTABLE_TRACKID,
                    SMARTIES_TRACKS_TABLE,
                    SMARTIES_TABLE,
                    SMARTIESTRACKSTABLE_SMARTIESID,
                    SMARTIESTABLE_ID,
                    SMARTIESTABLE_NAME,
                    escapedSmartiesNameLike);
}

// static
QString SmartiesStorage::formatQueryForTrackIdsWithSmarties() {
    return QStringLiteral(
            "SELECT DISTINCT %1 FROM %2 JOIN %3 ON %4=%5 ORDER BY %1")
            .arg(SMARTIESTRACKSTABLE_TRACKID,
                    SMARTIES_TRACKS_TABLE,
                    SMARTIES_TABLE,
                    SMARTIESTRACKSTABLE_SMARTIESID,
                    SMARTIESTABLE_ID);
}

SmartiesTrackSelectResult SmartiesStorage::selectSmartiesTracksSorted(
        SmartiesId smartiesId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:smartiesId ORDER BY %3")
                    .arg(SMARTIES_TRACKS_TABLE,
                            SMARTIESTRACKSTABLE_SMARTIESID,
                            SMARTIESTRACKSTABLE_TRACKID));
    query.bindValue(":smartiesId", smartiesId);
    if (query.execPrepared()) {
        return SmartiesTrackSelectResult(std::move(query));
    } else {
        return SmartiesTrackSelectResult();
    }
}

SmartiesTrackSelectResult SmartiesStorage::selectTrackSmartiesSorted(
        TrackId trackId) const {
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT * FROM %1 WHERE %2=:trackId ORDER BY %3")
                    .arg(SMARTIES_TRACKS_TABLE,
                            SMARTIESTRACKSTABLE_TRACKID,
                            SMARTIESTRACKSTABLE_SMARTIESID));
    query.bindValue(":trackId", trackId);
    if (query.execPrepared()) {
        return SmartiesTrackSelectResult(std::move(query));
    } else {
        return SmartiesTrackSelectResult();
    }
}

SmartiesSummarySelectResult SmartiesStorage::selectSmartiesWithTrackCount(
        const QList<TrackId>& trackIds) const {
    FwdSqlQuery query(m_database,
            mixxx::DbConnection::collateLexicographically(
                    QStringLiteral("SELECT *, "
                                   "(SELECT COUNT(*) FROM %1 WHERE %2.%3 = %1.%4 and "
                                   "%1.%5 in (%9)) AS %6, "
                                   "0 as %7 FROM %2 ORDER BY %8")
                            .arg(
                                    SMARTIES_TRACKS_TABLE,          // 1
                                    SMARTIES_TABLE,                 // 2
                                    SMARTIESTABLE_ID,               // 3
                                    SMARTIESTRACKSTABLE_SMARTIESID, // 4
                                    SMARTIESTRACKSTABLE_TRACKID,    // 5
                                    SMARTIESSUMMARY_TRACK_COUNT,    // 6
                                    SMARTIESSUMMARY_TRACK_DURATION, // 7
                                    SMARTIESTABLE_NAME,             // 8
                                    joinSqlStringList(trackIds))));

    if (query.execPrepared()) {
        return SmartiesSummarySelectResult(std::move(query));
    } else {
        return SmartiesSummarySelectResult();
    }
}

SmartiesTrackSelectResult SmartiesStorage::selectTracksSortedBySmartiesNameLike(
        const QString& smartiesNameLike) const {
    // TODO: Do SQL LIKE wildcards in smartiesNameLike need to be escaped?
    // Previously we used SqlLikeWildcardEscaper in the past for this
    // purpose. This utility class has become obsolete but could be
    // restored from the 2.3 branch if ever needed again.
    FwdSqlQuery query(m_database,
            QStringLiteral("SELECT %1,%2 FROM %3 "
                           "JOIN %4 ON %5 = %6 "
                           "WHERE %7 LIKE :smartiesNameLike "
                           "ORDER BY %1")
                    .arg(SMARTIESTRACKSTABLE_TRACKID,       // 1
                            SMARTIESTRACKSTABLE_SMARTIESID, // 2
                            SMARTIES_TRACKS_TABLE,          // 3
                            SMARTIES_TABLE,                 // 4
                            SMARTIESTABLE_ID,               // 5
                            SMARTIESTRACKSTABLE_SMARTIESID, // 6
                            SMARTIESTABLE_NAME));           // 7
    query.bindValue(":smartiesNameLike",
            QVariant(kSqlLikeMatchAll + smartiesNameLike + kSqlLikeMatchAll));

    if (query.execPrepared()) {
        return SmartiesTrackSelectResult(std::move(query));
    } else {
        return SmartiesTrackSelectResult();
    }
}

QSet<SmartiesId> SmartiesStorage::collectSmartiesIdsOfTracks(const QList<TrackId>& trackIds) const {
    // NOTE(uklotzde): One query per track id. This could be optimized
    // by querying for chunks of track ids and collecting the results.
    QSet<SmartiesId> trackSmarties;
    for (const auto& trackId : trackIds) {
        // NOTE(uklotzde): The query result does not need to be sorted by smarties id
        // here. But since the corresponding FK column is indexed the impact on the
        // performance should be negligible. By reusing an existing query we reduce
        // the amount of code and the number of prepared SQL queries.
        SmartiesTrackSelectResult smartiesTracks(selectTrackSmartiesSorted(trackId));
        while (smartiesTracks.next()) {
            DEBUG_ASSERT(smartiesTracks.trackId() == trackId);
            trackSmarties.insert(smartiesTracks.smartiesId());
        }
    }
    return trackSmarties;
}

bool SmartiesStorage::onInsertingSmarties(
        const Smarties& smarties,
        SmartiesId* pSmartiesId) {
    VERIFY_OR_DEBUG_ASSERT(!smarties.getId().isValid()) {
        kLogger.warning()
                << "Cannot insert smarties with a valid id:" << smarties.getId();
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
                    .arg(SMARTIES_TABLE,
                            SMARTIESTABLE_NAME,
                            SMARTIESTABLE_LOCKED,
                            SMARTIESTABLE_AUTODJ_SOURCE,
                            SMARTIESTABLE_SEARCH_INPUT, // 5
                            SMARTIESTABLE_SEARCH_SQL,
                            SMARTIESTABLE_CONDITION_1_FIELD,
                            SMARTIESTABLE_CONDITION_1_OPERATOR,
                            SMARTIESTABLE_CONDITION_1_VALUE,
                            SMARTIESTABLE_CONDITION_1_COMBINER, // 10
                            SMARTIESTABLE_CONDITION_2_FIELD,
                            SMARTIESTABLE_CONDITION_2_OPERATOR,
                            SMARTIESTABLE_CONDITION_2_VALUE,
                            SMARTIESTABLE_CONDITION_2_COMBINER,
                            SMARTIESTABLE_CONDITION_3_FIELD, // 15
                            SMARTIESTABLE_CONDITION_3_OPERATOR,
                            SMARTIESTABLE_CONDITION_3_VALUE,
                            SMARTIESTABLE_CONDITION_3_COMBINER,
                            SMARTIESTABLE_CONDITION_4_FIELD,
                            SMARTIESTABLE_CONDITION_4_OPERATOR, // 20
                            SMARTIESTABLE_CONDITION_4_VALUE,
                            SMARTIESTABLE_CONDITION_4_COMBINER,
                            SMARTIESTABLE_CONDITION_5_FIELD,
                            SMARTIESTABLE_CONDITION_5_OPERATOR,
                            SMARTIESTABLE_CONDITION_5_VALUE, // 25
                            SMARTIESTABLE_CONDITION_5_COMBINER,
                            SMARTIESTABLE_CONDITION_6_FIELD,
                            SMARTIESTABLE_CONDITION_6_OPERATOR,
                            SMARTIESTABLE_CONDITION_6_VALUE,
                            SMARTIESTABLE_CONDITION_6_COMBINER, // 30
                            SMARTIESTABLE_CONDITION_7_FIELD,
                            SMARTIESTABLE_CONDITION_7_OPERATOR,
                            SMARTIESTABLE_CONDITION_7_VALUE,
                            SMARTIESTABLE_CONDITION_7_COMBINER,
                            SMARTIESTABLE_CONDITION_8_FIELD, // 35
                            SMARTIESTABLE_CONDITION_8_OPERATOR,
                            SMARTIESTABLE_CONDITION_8_VALUE,
                            SMARTIESTABLE_CONDITION_8_COMBINER,
                            SMARTIESTABLE_CONDITION_9_FIELD,
                            SMARTIESTABLE_CONDITION_9_OPERATOR, // 40
                            SMARTIESTABLE_CONDITION_9_VALUE,
                            SMARTIESTABLE_CONDITION_9_COMBINER,
                            SMARTIESTABLE_CONDITION_10_FIELD,
                            SMARTIESTABLE_CONDITION_10_OPERATOR,
                            SMARTIESTABLE_CONDITION_10_VALUE, // 45
                            SMARTIESTABLE_CONDITION_10_COMBINER,
                            SMARTIESTABLE_CONDITION_11_FIELD,
                            SMARTIESTABLE_CONDITION_11_OPERATOR,
                            SMARTIESTABLE_CONDITION_11_VALUE,
                            SMARTIESTABLE_CONDITION_11_COMBINER, // 50
                            SMARTIESTABLE_CONDITION_12_FIELD,
                            SMARTIESTABLE_CONDITION_12_OPERATOR,
                            SMARTIESTABLE_CONDITION_12_VALUE,
                            SMARTIESTABLE_CONDITION_12_COMBINER)); // 54

    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    SmartiesQueryBinder queryBinder(query);
    queryBinder.bindName(":name", smarties);
    queryBinder.bindLocked(":locked", smarties);
    queryBinder.bindAutoDjSource(":autoDjSource", smarties);
    queryBinder.bindSearchInput(":searchInput", smarties);
    queryBinder.bindSearchSql(":searchSql", smarties);
    // Loop from 1 to 12 to bind all condition parameters dynamically
    // Call the corresponding bind methods on queryBinder, querybinder procedure
    // is longer but this procedure is a bit smaller... I don't know what's
    // right
    for (int i = 1; i <= 12; ++i) {
        QString suffix = QString::number(i);
        queryBinder.bindConditionField(QString(":condition%1_field").arg(suffix), smarties, i);
        queryBinder.bindConditionOperator(
                QString(":condition%1_operator").arg(suffix), smarties, i);
        queryBinder.bindConditionValue(QString(":condition%1_value").arg(suffix), smarties, i);
        queryBinder.bindConditionCombiner(
                QString(":condition%1_combiner").arg(suffix), smarties, i);
    }

    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }

    if (query.numRowsAffected() > 0) {
        DEBUG_ASSERT(query.numRowsAffected() == 1);
        if (pSmartiesId != nullptr) {
            *pSmartiesId = SmartiesId(query.lastInsertId());
            DEBUG_ASSERT(pSmartiesId->isValid());
        }
        return true;
    } else {
        return false;
    }
}

bool SmartiesStorage::onUpdatingSmarties(
        const Smarties& smarties) {
    VERIFY_OR_DEBUG_ASSERT(smarties.getId().isValid()) {
        kLogger.warning()
                << "Cannot update smarties without a valid id";
        return false;
    }
    FwdSqlQuery query(m_database,
            QString(
                    "UPDATE %1 "
                    "SET %2=:name,%3=:locked,%4=:autoDjSource, %5=:searchInput, %6=:searchSql "
                    "WHERE %7=:id")
                    .arg(
                            SMARTIES_TABLE,
                            SMARTIESTABLE_NAME,
                            SMARTIESTABLE_LOCKED,
                            SMARTIESTABLE_AUTODJ_SOURCE,
                            SMARTIESTABLE_SEARCH_INPUT,
                            SMARTIESTABLE_SEARCH_SQL,
                            SMARTIESTABLE_ID));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
        return false;
    }
    SmartiesQueryBinder queryBinder(query);
    queryBinder.bindId(":id", smarties);
    queryBinder.bindName(":name", smarties);
    queryBinder.bindLocked(":locked", smarties);
    queryBinder.bindAutoDjSource(":autoDjSource", smarties);
    queryBinder.bindSearchInput(":searchInput", smarties);
    queryBinder.bindSearchSql(":searchSql", smarties);
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    if (query.numRowsAffected() > 0) {
        VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
            kLogger.warning()
                    << "Updated multiple smarties with the same id" << smarties.getId();
        }
        return true;
    } else {
        kLogger.warning()
                << "Cannot update non-existent smarties with id" << smarties.getId();
        return false;
    }
}

bool SmartiesStorage::onDeletingSmarties(
        SmartiesId smartiesId) {
    VERIFY_OR_DEBUG_ASSERT(smartiesId.isValid()) {
        kLogger.warning()
                << "Cannot delete smarties without a valid id";
        return false;
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(SMARTIES_TRACKS_TABLE, SMARTIESTRACKSTABLE_SMARTIESID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", smartiesId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() <= 0) {
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Deleting empty smarties with id"
                        << smartiesId;
            }
        }
    }
    {
        FwdSqlQuery query(m_database,
                QStringLiteral("DELETE FROM %1 WHERE %2=:id")
                        .arg(SMARTIES_TABLE, SMARTIESTABLE_ID));
        VERIFY_OR_DEBUG_ASSERT(query.isPrepared()) {
            return false;
        }
        query.bindValue(":id", smartiesId);
        VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() > 0) {
            VERIFY_OR_DEBUG_ASSERT(query.numRowsAffected() <= 1) {
                kLogger.warning()
                        << "Deleted multiple smarties with the same id" << smartiesId;
            }
            return true;
        } else {
            kLogger.warning()
                    << "Cannot delete non-existent smarties with id" << smartiesId;
            return false;
        }
    }
}

bool SmartiesStorage::onAddingSmartiesTracks(
        SmartiesId smartiesId,
        const QList<TrackId>& trackIds) {
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "INSERT OR IGNORE INTO %1 (%2, %3) "
                    "VALUES (:smartiesId,:trackId)")
                    .arg(
                            SMARTIES_TRACKS_TABLE,
                            SMARTIESTRACKSTABLE_SMARTIESID,
                            SMARTIESTRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":smartiesId", smartiesId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track is already in smarties
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not added to smarties" << smartiesId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool SmartiesStorage::onRemovingSmartiesTracks(
        SmartiesId smartiesId,
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): We remove tracks in a loop
    // analogously to adding tracks (see above).
    FwdSqlQuery query(m_database,
            QStringLiteral(
                    "DELETE FROM %1 "
                    "WHERE %2=:smartiesId AND %3=:trackId")
                    .arg(
                            SMARTIES_TRACKS_TABLE,
                            SMARTIESTRACKSTABLE_SMARTIESID,
                            SMARTIESTRACKSTABLE_TRACKID));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":smartiesId", smartiesId);
    for (const auto& trackId : trackIds) {
        query.bindValue(":trackId", trackId);
        if (!query.execPrepared()) {
            return false;
        }
        if (query.numRowsAffected() == 0) {
            // track not found in smarties
            if (kLogger.debugEnabled()) {
                kLogger.debug()
                        << "Track" << trackId
                        << "not removed from smarties" << smartiesId;
            }
        } else {
            DEBUG_ASSERT(query.numRowsAffected() == 1);
        }
    }
    return true;
}

bool SmartiesStorage::onPurgingTracks(
        const QList<TrackId>& trackIds) {
    // NOTE(uklotzde): Remove tracks from smarties one-by-one.
    // This might be optimized by deleting multiple track ids
    // at once in chunks with a maximum size.
    FwdSqlQuery query(m_database,
            QStringLiteral("DELETE FROM %1 WHERE %2=:trackId")
                    .arg(SMARTIES_TRACKS_TABLE, SMARTIESTRACKSTABLE_TRACKID));
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
