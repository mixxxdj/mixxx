#pragma once

#include <QList>
#include <memory>

#include "tagging/customtags.h"
#include "track/trackid.h"
#include "util/db/fwdsqlquery.h"

namespace mixxx {

class TrackCustomTagsStorage : public QObject {
    Q_OBJECT
  public:
    static const QString kColumnFacet;
    static const QString kColumnLabel;
    static const QString kColumnScore;

    // TODO: Move into a generic SQL query utility class or module
    static QString joinTrackIdList(
            const TrackIdList& trackIds);

    /// Build a filter (AND) from a list of facet/label pairs.
    static QString buildTagFilter(
            const QSqlDatabase& database,
            const QList<std::pair<TagFacet, TagLabel>>& facetedLabelList);

    /// Build a SELECT statement for the track id that is needed
    /// for using a tag filter in subselects.
    static QString buildTrackIdSelect(
            const QString& tagFilter);

    TrackCustomTagsStorage(
            const QSqlDatabase& database,
            QObject* parent = nullptr);
    ~TrackCustomTagsStorage() override = default;

    static void cleanup(
            const QSqlDatabase& database);

    std::optional<mixxx::CustomTags> loadSingleTrack(
            const TrackId& trackId) const;

    bool insertSingleTrack(
            const TrackId& trackId,
            const mixxx::CustomTags& customTags) const;

    bool replaceSingleTrack(
            const TrackId& trackId,
            const mixxx::CustomTags& customTags) const;

    bool deleteSingleTrack(
            const TrackId& trackId) const;
    bool deleteMultipleTracks(
            const QString& trackIdList) const;
    bool deleteMultipleTracks(
            const TrackIdList& trackIds) const {
        return deleteMultipleTracks(
                joinTrackIdList(trackIds));
    }

    // Empty track id list = all tracks
    QList<std::pair<TagFacet, qulonglong>> findMostFrequentFacets(
            const QString& trackIdList = QString()) const;
    QList<std::pair<TagFacet, qulonglong>> findMostFrequentFacets(
            const TrackIdList& trackIds) const {
        return findMostFrequentFacets(
                joinTrackIdList(trackIds));
    }

    // Empty track id list = all tracks
    QList<std::pair<TagLabel, qulonglong>> findMostFrequentLabelsOfFacet(
            const TagFacet& tagFacet,
            const QString& trackIdList = QString()) const;
    QList<std::pair<TagLabel, qulonglong>> findMostFrequentLabelsOfFacet(
            const TagFacet& tagFacet,
            const TrackIdList& trackIds) const {
        return findMostFrequentLabelsOfFacet(
                tagFacet,
                joinTrackIdList(trackIds));
    }

    // Empty track ids/list = all tracks
    // Returns the number of added tags or -1 on error
    int mergeFacetsAndLabelsInto(
            CustomTags* pCustomTags,
            const QString& trackIdList = QString()) const;
    int mergeFacetsAndLabelsInto(
            CustomTags* pCustomTags,
            const TrackIdList& trackIds) const {
        return mergeFacetsAndLabelsInto(pCustomTags,
                joinTrackIdList(trackIds));
    }

    qulonglong countTags(
            const QString& trackIdList,
            const TagLabel& tagLabel,
            const TagFacet& tagFacet = TagFacet()) const;
    qulonglong countTags(
            const TrackIdList& trackIds,
            const TagLabel& tagLabel,
            const TagFacet& tagFacet = TagFacet()) const {
        return countTags(
                joinTrackIdList(trackIds),
                tagLabel,
                tagFacet);
    }

  private:
    const QSqlDatabase m_database;

    bool prepareQueriesLazily() const;

    struct PreparedQueries {
        FwdSqlQuery loadSingleTrack;
        FwdSqlQuery insertSingleTrack;
        FwdSqlQuery deleteSingleTrack;
    };
    mutable std::unique_ptr<PreparedQueries> m_preparedQueries;
};

} // namespace mixxx
