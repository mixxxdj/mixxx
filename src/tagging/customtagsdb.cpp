#include "tagging/customtagsdb.h"

#include "library/dao/trackschema.h"
#include "util/assert.h"
#include "util/db/sqlstringformatter.h"
#include "util/logger.h"

namespace mixxx {

//static
const QString TrackCustomTagsStorage::kColumnFacet = QStringLiteral("facet");

//static
const QString TrackCustomTagsStorage::kColumnLabel = QStringLiteral("label");

//static
const QString TrackCustomTagsStorage::kColumnScore = QStringLiteral("score");

namespace {

Logger kLogger("tagging.customtags.db");

const QString kTable = QStringLiteral("track_custom_tags");
const QString kColumnTrackId = QStringLiteral("track_id");

bool purgeOrphanedRecords(
        const QSqlDatabase& database) {
    // Remove location from track_locations table
    FwdSqlQuery query(database,
            QStringLiteral("DELETE FROM %1 WHERE %2 NOT IN (SELECT %4 FROM %3)")
                    .arg(
                            kTable,
                            kColumnTrackId,
                            LIBRARY_TABLE,
                            LIBRARYTABLE_ID));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared() && !query.hasError()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    return true;
}

bool fixEmptyTagFacets(
        const QSqlDatabase& database) {
    // A bug during development created records with empty facets
    // instead of NULL values.
    FwdSqlQuery query(database,
            QStringLiteral("UPDATE %1 SET %2=NULL WHERE %2=''")
                    .arg(
                            kTable,
                            TrackCustomTagsStorage::kColumnFacet));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared() && !query.hasError()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    return true;
}

bool fixEmptyTagLabels(
        const QSqlDatabase& database) {
    // A bug during development created records with empty labels
    // instead of NULL values.
    FwdSqlQuery query(database,
            QStringLiteral("UPDATE %1 SET %2=NULL WHERE %2=''")
                    .arg(
                            kTable,
                            TrackCustomTagsStorage::kColumnLabel));
    VERIFY_OR_DEBUG_ASSERT(query.isPrepared() && !query.hasError()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return false;
    }
    return true;
}

} // namespace

//static
QString TrackCustomTagsStorage::joinTrackIdList(
        const TrackIdList& trackIds) {
    QString result;
    result.reserve(trackIds.size() * (7 + 1)); // 7 digits + ',' character
    for (const auto& trackId : trackIds) {
        if (!result.isEmpty()) {
            result += QChar(',');
        }
        result.append(trackId.toString());
    }
    return result;
}

//static
QString TrackCustomTagsStorage::buildTagFilter(
        const QSqlDatabase& database,
        const QList<std::pair<TagFacet, TagLabel>>& facetedLabelList) {
    QString tagFilter;
    for (const auto& facetLabelPair : facetedLabelList) {
        const auto& facet = facetLabelPair.first;
        const auto& label = facetLabelPair.second;
        QString filterTerm;
        if (!facet.isEmpty()) {
            filterTerm = QString("%1=%2").arg(
                    kColumnFacet,
                    SqlStringFormatter::format(
                            database,
                            facet));
        }
        if (!label.isEmpty()) {
            filterTerm = QString("%1=%2").arg(
                    kColumnLabel,
                    SqlStringFormatter::format(
                            database,
                            label));
        }
        VERIFY_OR_DEBUG_ASSERT(!filterTerm.isEmpty()) {
            continue;
        }
        if (tagFilter.isEmpty()) {
            tagFilter = filterTerm;
        } else {
            tagFilter += QStringLiteral(" AND ") + filterTerm;
        }
    }
    return tagFilter;
}

//static
QString TrackCustomTagsStorage::buildTrackIdSelect(
        const QString& tagFilter) {
    VERIFY_OR_DEBUG_ASSERT(!tagFilter.isEmpty()) {
        return QString();
    }
    return QString(
            QStringLiteral("SELECT %2 from %1 WHERE %3"))
            .arg(kTable, kColumnTrackId, tagFilter);
}

TrackCustomTagsStorage::TrackCustomTagsStorage(
        const QSqlDatabase& database,
        QObject* parent)
        : QObject(parent),
          m_database(database) {
}

//static
void TrackCustomTagsStorage::cleanup(
        const QSqlDatabase& database) {
    VERIFY_OR_DEBUG_ASSERT(purgeOrphanedRecords(database)) {
        kLogger.warning()
                << "Failed to purge orphaned records";
    }
    VERIFY_OR_DEBUG_ASSERT(fixEmptyTagFacets(database)) {
        kLogger.warning()
                << "Failed to fix empty tag facets";
    }
    VERIFY_OR_DEBUG_ASSERT(fixEmptyTagLabels(database)) {
        kLogger.warning()
                << "Failed to fix empty tag labels";
    }
}

bool TrackCustomTagsStorage::prepareQueriesLazily() const {
    VERIFY_OR_DEBUG_ASSERT(m_database.isOpen()) {
        m_preparedQueries.reset();
        return false;
    }
    if (m_preparedQueries) {
        return true;
    }
    auto preparedQueries = std::make_unique<PreparedQueries>();
    preparedQueries->loadSingleTrack = FwdSqlQuery(
            m_database,
            QStringLiteral("SELECT %3,%4,%5 FROM %1 WHERE %2=:%2")
                    .arg(
                            kTable,
                            kColumnTrackId,
                            kColumnFacet,
                            kColumnLabel,
                            kColumnScore));
    preparedQueries->insertSingleTrack = FwdSqlQuery(
            m_database,
            QStringLiteral("INSERT INTO %1(%2,%3,%4,%5) VALUES(:%2,:%3,:%4,:%5)")
                    .arg(
                            kTable,
                            kColumnTrackId,
                            kColumnFacet,
                            kColumnLabel,
                            kColumnScore));
    preparedQueries->deleteSingleTrack = FwdSqlQuery(
            m_database,
            QStringLiteral("DELETE FROM %1 WHERE %2=:%2")
                    .arg(
                            kTable,
                            kColumnTrackId));
    m_preparedQueries = std::move(preparedQueries);
    return true;
}

std::optional<CustomTags> TrackCustomTagsStorage::loadSingleTrack(
        const TrackId& trackId) const {
    VERIFY_OR_DEBUG_ASSERT(
            prepareQueriesLazily() &&
            m_preparedQueries->loadSingleTrack.isPrepared() &&
            !m_preparedQueries->loadSingleTrack.hasError()) {
        return std::nullopt;
    }
    auto query = m_preparedQueries->loadSingleTrack;
    query.bindValue(
            QChar(':') + kColumnTrackId,
            trackId.toVariant());
    VERIFY_OR_DEBUG_ASSERT(query.execPrepared()) {
        return std::nullopt;
    }
    CustomTags customTags;
    const auto facetIndex = query.fieldIndex(kColumnFacet);
    const auto labelIndex = query.fieldIndex(kColumnLabel);
    const auto scoreIndex = query.fieldIndex(kColumnScore);
    while (query.next()) {
        auto scoreOk = false;
        auto scoreValue =
                query.fieldValue(scoreIndex).toDouble(&scoreOk);
        VERIFY_OR_DEBUG_ASSERT(scoreOk &&
                TagScore::isValidValue(scoreValue)) {
            scoreValue = TagScore();
        }
        auto label = TagLabel(
                query.fieldValue(labelIndex).toString());
        auto facet = TagFacet(
                query.fieldValue(facetIndex).toString());
        customTags.addOrReplaceTag(
                Tag(
                        std::move(label),
                        TagScore(scoreValue)),
                facet);
    }
    return customTags;
}

bool TrackCustomTagsStorage::insertSingleTrack(
        const TrackId& trackId,
        const CustomTags& customTags) const {
    VERIFY_OR_DEBUG_ASSERT(
            prepareQueriesLazily() &&
            m_preparedQueries->insertSingleTrack.isPrepared() &&
            !m_preparedQueries->insertSingleTrack.hasError()) {
        return false;
    }
    auto query = m_preparedQueries->insertSingleTrack;
    query.bindValue(
            QChar(':') + kColumnTrackId,
            trackId.toVariant());
    for (auto i = customTags.getFacetedTags().begin();
            i != customTags.getFacetedTags().end();
            ++i) {
        query.bindValue(
                QChar(':') + kColumnFacet,
                i.key().value());
        for (auto j = i.value().begin();
                j != i.value().end();
                ++j) {
            query.bindValue(
                    QChar(':') + kColumnLabel,
                    j.key().value());
            query.bindValue(
                    QChar(':') + kColumnScore,
                    j.value().value());
            VERIFY_OR_DEBUG_ASSERT(
                    query.execPrepared()) {
                return false;
            }
        }
    }
    return true;
}

bool TrackCustomTagsStorage::replaceSingleTrack(
        const TrackId& trackId,
        const CustomTags& customTags) const {
    return deleteSingleTrack(
                   trackId) &&
            insertSingleTrack(
                    trackId,
                    customTags);
}

bool TrackCustomTagsStorage::deleteSingleTrack(
        const TrackId& trackId) const {
    VERIFY_OR_DEBUG_ASSERT(
            prepareQueriesLazily() &&
            m_preparedQueries->deleteSingleTrack.isPrepared() &&
            !m_preparedQueries->deleteSingleTrack.hasError()) {
        return false;
    }
    auto query = m_preparedQueries->deleteSingleTrack;
    query.bindValue(
            QChar(':') + kColumnTrackId,
            trackId.toVariant());
    VERIFY_OR_DEBUG_ASSERT(
            query.execPrepared()) {
        return false;
    }
    return true;
}

bool TrackCustomTagsStorage::deleteMultipleTracks(
        const QString& trackIdList) const {
    auto query = FwdSqlQuery(
            m_database,
            QString(
                    QStringLiteral("DELETE FROM %1 WHERE %2 IN (%3)"))
                    .arg(
                            kTable,
                            kColumnTrackId,
                            trackIdList));
    VERIFY_OR_DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError()) {
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(
            query.execPrepared()) {
        return false;
    }
    return true;
}

QList<std::pair<TagFacet, qulonglong>> TrackCustomTagsStorage::findMostFrequentFacets(
        const QString& trackIdList) const {
    QList<std::pair<TagFacet, qulonglong>> result;
    auto query = FwdSqlQuery(
            m_database,
            QStringLiteral("SELECT %1,COUNT(%1) FROM %2")
                            .arg(
                                    kColumnFacet,
                                    kTable) +
                    (trackIdList.isEmpty()
                                    ? QStringLiteral(" WHERE %1 IS NOT NULL")
                                              .arg(
                                                      kColumnTrackId)
                                    : QStringLiteral(" WHERE %1 IN (%2)")
                                              .arg(
                                                      kColumnTrackId,
                                                      trackIdList)) +
                    QStringLiteral(" GROUP BY %1 ORDER BY COUNT(%1) DESC")
                            .arg(
                                    kColumnFacet));
    VERIFY_OR_DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError()) {
        return result;
    }
    VERIFY_OR_DEBUG_ASSERT(
            query.execPrepared()) {
        return result;
    }
    while (query.next()) {
        bool ok = false;
        Q_UNUSED(ok); // only used in DEBUG_ASSERT
        const auto count = query.fieldValue(1).toULongLong(&ok);
        DEBUG_ASSERT(ok);
        const auto facet = TagFacet(query.fieldValue(0).toString());
        result.append(std::make_pair(facet, count));
    }
    return result;
}

QList<std::pair<TagLabel, qulonglong>> TrackCustomTagsStorage::findMostFrequentLabelsOfFacet(
        const TagFacet& tagFacet,
        const QString& trackIdList) const {
    QList<std::pair<TagLabel, qulonglong>> result;
    auto query = FwdSqlQuery(
            m_database,
            QStringLiteral("SELECT %1,COUNT(%1) FROM %2")
                            .arg(
                                    kColumnLabel,
                                    kTable) +
                    (trackIdList.isEmpty()
                                    ? QStringLiteral(" WHERE %1 IS NOT NULL")
                                              .arg(
                                                      kColumnTrackId)
                                    : QStringLiteral(" WHERE %1 IN (%2)")
                                              .arg(
                                                      kColumnTrackId,
                                                      trackIdList)) +
                    (tagFacet.isEmpty()
                                    ? QStringLiteral(" AND %1 IS NULL")
                                    : QStringLiteral(" AND %1=:%1"))
                            .arg(kColumnFacet) +
                    QStringLiteral(" GROUP BY %1 ORDER BY COUNT(%1) DESC")
                            .arg(
                                    kColumnLabel));
    VERIFY_OR_DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError()) {
        return result;
    }
    if (!tagFacet.isEmpty()) {
        query.bindValue(
                QChar(':') + kColumnFacet,
                tagFacet.value());
    }
    VERIFY_OR_DEBUG_ASSERT(
            query.execPrepared()) {
        return result;
    }
    while (query.next()) {
        bool ok = false;
        Q_UNUSED(ok); // only used in DEBUG_ASSERT
        const auto count = query.fieldValue(1).toULongLong(&ok);
        DEBUG_ASSERT(ok);
        const auto label = TagLabel(query.fieldValue(0).toString());
        result.append(std::make_pair(label, count));
    }
    return result;
}

int TrackCustomTagsStorage::mergeFacetsAndLabelsInto(
        CustomTags* pCustomTags,
        const QString& trackIdList) const {
    VERIFY_OR_DEBUG_ASSERT(pCustomTags) {
        return -1;
    }
    auto query = FwdSqlQuery(
            m_database,
            QStringLiteral("SELECT %1,%2 FROM %3")
                            .arg(
                                    kColumnFacet,
                                    kColumnLabel,
                                    kTable) +
                    (trackIdList.isEmpty()
                                    ? QStringLiteral(" WHERE %1 IS NOT NULL")
                                              .arg(
                                                      kColumnTrackId)
                                    : QStringLiteral(" WHERE %1 IN (%2)")
                                              .arg(
                                                      kColumnTrackId,
                                                      trackIdList)) +
                    QStringLiteral(" GROUP BY %1,%2")
                            .arg(
                                    kColumnFacet,
                                    kColumnLabel));
    VERIFY_OR_DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError()) {
        return -1;
    }
    VERIFY_OR_DEBUG_ASSERT(
            query.execPrepared()) {
        return -1;
    }
    int added = 0;
    while (query.next()) {
        const auto facet = TagFacet(query.fieldValue(0).toString());
        const auto label = TagLabel(query.fieldValue(1).toString());
        DEBUG_ASSERT(!(facet.isEmpty() && label.isEmpty()));
        if (!pCustomTags->containsTag(label, facet)) {
            kLogger.debug()
                    << "Adding"
                    << facet
                    << "->"
                    << label;
            pCustomTags->addOrReplaceTag(Tag(label), facet);
            ++added;
        }
    }
    return added;
}

qulonglong TrackCustomTagsStorage::countTags(
        const QString& trackIdList,
        const TagLabel& tagLabel,
        const TagFacet& tagFacet) const {
    auto query = FwdSqlQuery(
            m_database,
            QStringLiteral("SELECT COUNT(*) FROM %1")
                            .arg(
                                    kTable) +
                    (trackIdList.isEmpty()
                                    ? QStringLiteral(" WHERE %1 IS NOT NULL")
                                              .arg(
                                                      kColumnTrackId)
                                    : QStringLiteral(" WHERE %1 IN (%2)")
                                              .arg(
                                                      kColumnTrackId,
                                                      trackIdList)) +
                    (tagLabel.isEmpty()
                                    ? QStringLiteral(" AND %1 IS NULL")
                                    : QStringLiteral(" AND %1=:%1"))
                            .arg(kColumnLabel) +
                    (tagFacet.isEmpty()
                                    ? QStringLiteral(" AND %1 IS NULL")
                                    : QStringLiteral(" AND %1=:%1"))
                            .arg(kColumnFacet));
    VERIFY_OR_DEBUG_ASSERT(
            query.isPrepared() &&
            !query.hasError()) {
        return false;
    }
    if (!tagLabel.isEmpty()) {
        query.bindValue(
                QChar(':') + kColumnLabel,
                tagLabel.value());
    }
    if (!tagFacet.isEmpty()) {
        query.bindValue(
                QChar(':') + kColumnFacet,
                tagFacet.value());
    }
    VERIFY_OR_DEBUG_ASSERT(
            query.execPrepared()) {
        return false;
    }
    while (query.next()) {
        bool ok = false;
        Q_UNUSED(ok); // only used in DEBUG_ASSERT
        const auto count = query.fieldValue(0).toULongLong(&ok);
        DEBUG_ASSERT(ok);
        DEBUG_ASSERT(!query.next()); // single row result
        return count;
    }
    DEBUG_ASSERT(!"unreachable");
    return 0;
}

} // namespace mixxx
