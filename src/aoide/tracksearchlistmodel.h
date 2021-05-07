#pragma once

#include <QAbstractListModel>
#include <QJsonArray>
#include <optional>

#include "aoide/web/searchcollectedtrackstask.h"
#include "util/qt.h"

namespace aoide {

class Subsystem;

struct TrackSearchListParams {
    QJsonObject baseQuery;
    TrackSearchOverlayFilter overlayFilter;
    QStringList searchTerms;

    void normalize() {
        searchTerms.sort();
        searchTerms.removeDuplicates();
    }
};

inline bool operator==(const TrackSearchListParams& lhs, const TrackSearchListParams& rhs) {
    return lhs.baseQuery == rhs.baseQuery &&
            lhs.overlayFilter == rhs.overlayFilter &&
            lhs.searchTerms == rhs.searchTerms;
}

inline bool operator!=(const TrackSearchListParams& lhs, const TrackSearchListParams& rhs) {
    return !(lhs == rhs);
}

struct TrackSearchListItem final {
    TrackSearchListItem() = default;
    explicit TrackSearchListItem(json::TrackEntity&& entity);
    TrackSearchListItem(const TrackSearchListItem&) = default;
    TrackSearchListItem(TrackSearchListItem&&) = default;
    /*non-virtual*/ ~TrackSearchListItem() = default;

    TrackSearchListItem& operator=(const TrackSearchListItem&) = default;
    TrackSearchListItem& operator=(TrackSearchListItem&&) = default;

    // Entity without any tags
    json::TrackEntity entity;

    // Separately managed facets for convenient and quick accessibility
    mixxx::Facets facets;
};

class TrackSearchListModel : public QAbstractListModel {
    Q_OBJECT

  public:
    static constexpr quint64 kDefaultPageSize = 200;

    enum ItemDataRole {
        ItemRole = Qt::UserRole,           // aoide::TrackSearchListItem
        IdRole,                            // QString
        EntityUidRole,                     // aoide::json::EntityUid
        MediaSourcePathRole,               // QString
        MediaSourcePathUrlRole,            // QUrl
        MediaSourceContentTypeRole,        // QString
        MediaSourceCollectedAtRole,        // QDateTime
        MediaSourceSynchronizedAtRole,     // QDateTime
        AudioContentDurationRole,          // mixxx::Duration
        AudioContentDurationMillisRole,    // double
        AudioContentChannelCountRole,      // mixxx::audio::ChannelCount
        AudioContentChannelCountValueRole, // mixxx::audio::ChannelCount::value_t
        AudioContentSampleRateRole,        // mixxx::audio::SampleRate
        AudioContentSampleRateHzRole,      // mixxx::audio::SampleRate::value_t
        AudioContentBitrateRole,           // mixxx::audio::Bitrate
        AudioContentBitrateBpsRole,        // mixxx::audio::Bitrate::value_t
        AudioContentReplayGainRole,        // mixxx::ReplayGain
        AudioContentReplayGainRatioRole,   // double
        MusicMetricsBpmRole,               // mixxx::Bpm
        MusicMetricsBpmValueRole,          // mixxx::Bpm::value_t
        MusicMetricsBpmLockedRole,         // bool
        MusicMetricsChromaticKeyRole,      // mixxx::track::io::key::ChromaticKey
        MusicMetricsKeyLockedRole,         // bool
        TrackArtistRole,                   // QString
        TrackTitleRole,                    // QString
        AlbumArtistRole,                   // QString
        AlbumTitleRole,                    // QString
        ComposerRole,                      // QString
        GenresRole,                        // QStringList
        GenreTagsRole,                     // mixxx::TagVector
        MoodsRole,                         // QStringList
        MoodTagsRole,                      // mixxx::TagVector
        CommentRole,                       // QString
        GroupingRole,                      // QString
        ReleasedAtRole,                    // QDateTime
        TrackNumbersRole,                  // QString
        DiscNumbersRole,                   // QString
        RgbColorRole,                      // mixxx::RgbColor
        QColorRole,                        // QColor
        LastPlayedAtRole,                  // QDateTime
        TimesPlayedRole,                   // int
    };
    Q_ENUM(ItemDataRole)

    explicit TrackSearchListModel(
            Subsystem* subsystem,
            QObject* parent = nullptr);
    ~TrackSearchListModel() override;

    Q_PROPERTY(quint64 pageSize MEMBER m_pageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool pending READ isPending STORED false NOTIFY pendingChanged)

    const std::optional<TrackSearchListParams>& params() const {
        return m_params;
    }

    bool isPending() const {
        return m_pendingTask;
    }

    ///////////////////////////////////////////////////////
    // Inherited from QAbstractItemModel
    ///////////////////////////////////////////////////////

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(
            const QModelIndex& parent = QModelIndex()) const final;

    QVariant data(
            const QModelIndex& index,
            int role = Qt::DisplayRole) const override;

    bool canFetchMore(
            const QModelIndex& parent) const final;
    void fetchMore(
            const QModelIndex& parent) final;

  signals:
    void pageSizeChanged(quint64 pageSize);
    void pendingChanged(bool pending);
    void paramsChanged(const std::optional<TrackSearchListParams>& params);

  public slots:
    void setParams(
            std::optional<TrackSearchListParams> params);

    void abortPendingTask();

  private slots:
    void onPendingTaskSucceeded(
            const QJsonArray& nextRows);
    void onPendingTaskDestroyed(
            QObject* obj);

  private:
    Pagination nextPagination() const;

    void resetModel();
    void loadNextPage(
            const TrackSearchListParams& params);

    const mixxx::SafeQPointer<Subsystem> m_subsystem;

    quint64 m_pageSize;

    std::optional<TrackSearchListParams> m_params;

    QVector<TrackSearchListItem> m_rowItems;

    bool m_canFetchMore;

    TrackSearchListParams m_pendingParams;
    Pagination m_pendingPagination;

    mixxx::SafeQPointer<SearchCollectedTracksTask> m_pendingTask;
};

} // namespace aoide

Q_DECLARE_METATYPE(aoide::TrackSearchListItem);
Q_DECLARE_METATYPE(aoide::TrackSearchListParams);
Q_DECLARE_METATYPE(std::optional<aoide::TrackSearchListParams>);
