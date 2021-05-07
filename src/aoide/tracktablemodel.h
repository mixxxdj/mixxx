#pragma once

#include <QHash>
#include <QList>
#include <QPointer>
#include <QSet>

#include "aoide/subsystem.h"
#include "aoide/tracksearchoverlayfilter.h"
#include "library/basetracktablemodel.h"
#include "util/qt.h"

namespace aoide {

class TrackTableModel : public BaseTrackTableModel {
    Q_OBJECT

  public:
    struct RowItem final {
        RowItem() = default;
        RowItem(json::TrackEntity&& entity,
                const mixxx::TaggingConfig& taggingConfig);
        RowItem(const RowItem&) = default;
        RowItem(RowItem&&) = default;
        /*non-virtual*/ ~RowItem() = default;

        RowItem& operator=(const RowItem&) = default;
        RowItem& operator=(RowItem&&) = default;

        json::TrackEntity entity;
        mixxx::Facets facets;
        // Text fields from multi-valued custom tags
        QString genre;
        QString mood;
        // Text fields from single-valued custom tags
        QString comment;
        QString grouping;
    };

    TrackTableModel(
            TrackCollectionManager* pTrackCollectionManager,
            Subsystem* subsystem,
            QObject* parent = nullptr);
    ~TrackTableModel() override;

    const RowItem& rowItem(
            const QModelIndex& index) const;

    const QString& searchText() const {
        return m_searchText;
    }

    ///////////////////////////////////////////////////////
    // Inherited from QAbstractItemModel
    ///////////////////////////////////////////////////////

    int rowCount(
            const QModelIndex& parent = {}) const final;

    bool canFetchMore(
            const QModelIndex& parent) const final;
    void fetchMore(const QModelIndex& parent) final;

    Qt::ItemFlags flags(
            const QModelIndex& index) const override;

    ///////////////////////////////////////////////////////
    // Inherited from TrackModel
    ///////////////////////////////////////////////////////

    Capabilities getCapabilities() const final;

    bool isColumnInternal(
            int column) final;

    TrackPointer getTrack(
            const QModelIndex& index) const final;
    TrackId getTrackId(
            const QModelIndex& index) const final;
    QString getTrackLocation(
            const QModelIndex& index) const final;
    CoverInfo getCoverInfo(
            const QModelIndex& index) const final;
    QImage getCoverThumbnail(
            const QModelIndex& index) const final;

    const QVector<int> getTrackRows(
            TrackId trackId) const final;

    void search(
            const QString& searchText,
            const QString& extraFilter = {}) final;
    const QString currentSearch() const final;

    void select() final;

    SortColumnId sortColumnIdFromColumnIndex(int index) const final {
        Q_UNUSED(index)
        return SortColumnId::Invalid; // sorting is not yet supported
    }

    int columnIndexFromSortColumnId(TrackModel::SortColumnId sortColumn) const final {
        Q_UNUSED(sortColumn)
        return -1; // sorting is not yet supported
    }

  signals:
    void doubleClicked(
            TrackPointer track);
    void rightClickPressed(
            TrackPointer track);
    void rightClickReleased();

  public slots:
    void reset();

    void searchTracks(
            const TrackSearchOverlayFilter& overlayFilter,
            const QString& searchText) {
        searchTracks(m_baseQuery, overlayFilter, searchText);
    }
    void searchTracks(
            const QJsonObject& baseQuery,
            const TrackSearchOverlayFilter& overlayFilter,
            const QString& searchText);

  private slots:
    void slotSearchTracksSucceeded(
            const QJsonArray& searchResults);

    void slotTracksChangedOrRemoved(
            QSet<TrackId> trackIds);

  protected:
    QVariant rawSiblingValue(
            const QModelIndex& index,
            ColumnCache::Column siblingField) const override;

    QVariant roleValue(
            const QModelIndex& index,
            QVariant&& rawValue,
            int role) const override;

    bool setTrackValueForColumn(
            const TrackPointer& pTrack,
            int column,
            const QVariant& value,
            int role) override;

  private:
    void clearRows();
    const RowItem& rowItem(
            int row) const;

    mixxx::FileInfo getTrackFileInfoByRow(
            int row) const;
    TrackRef getTrackFileRefByRow(
            int row) const {
        return TrackRef::fromFileInfo(getTrackFileInfoByRow(row));
    }
    TrackRef getTrackFileRef(
            const QModelIndex& index) const;

    TrackId getTrackIdByRow(
            int row) const;

    void startNewSearch(
            const Pagination& pagination);
    void abortPendingSearch();

    mixxx::FileInfo rowItemFileInfo(
            const RowItem& rowItem) const;

    const mixxx::SafeQPointer<Subsystem> m_subsystem;

    int m_rowsPerPage;

    QJsonObject m_baseQuery;
    TrackSearchOverlayFilter m_searchOverlayFilter;
    QString m_searchText;
    QStringList m_searchTerms;

    QString m_collectionUid;
    mixxx::SafeQPointer<SearchCollectedTracksTask> m_pendingSearchTask;
    bool m_canFetchMore;
    int m_pendingRequestFirstRow;
    int m_pendingRequestLastRow;

    struct RowItemPage {
        RowItemPage(int firstRow, QVector<RowItem>&& rowItems)
                : m_firstRow(firstRow),
                  m_rowItems(std::move(rowItems)) {
        }
        int m_firstRow;
        QVector<RowItem> m_rowItems;
    };

    QList<RowItemPage> m_rowItemPages;

    mutable QSet<int> m_staleRows;
    mutable QSet<TrackId> m_staleTrackIds;
    mutable QHash<TrackId, int> m_trackIdRowCache;

    int findRowItemPageIndex(
            int row) const;

    void invalidateRow(
            int row) const;
    void invalidateTrackId(
            TrackId trackId) const;
    void emitRowDataChanged(
            int row);

    void clearRowItems();
};

} // namespace aoide
