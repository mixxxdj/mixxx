#pragma once

#include <QStandardItemModel>

#include "library/trackmodel.h"
#include "library/browse/browsethread.h"

//constants
constexpr int COLUMN_PREVIEW = 0;
constexpr int COLUMN_FILENAME = 1;
constexpr int COLUMN_ARTIST = 2;
constexpr int COLUMN_TITLE = 3;
constexpr int COLUMN_ALBUM = 4;
constexpr int COLUMN_TRACK_NUMBER = 5;
constexpr int COLUMN_YEAR = 6;
constexpr int COLUMN_GENRE = 7;
constexpr int COLUMN_COMPOSER = 8;
constexpr int COLUMN_COMMENT = 9;
constexpr int COLUMN_DURATION = 10;
constexpr int COLUMN_BPM = 11;
constexpr int COLUMN_KEY = 12;
constexpr int COLUMN_TYPE = 13;
constexpr int COLUMN_BITRATE = 14;
constexpr int COLUMN_NATIVELOCATION = 15;
constexpr int COLUMN_ALBUMARTIST = 16;
constexpr int COLUMN_GROUPING = 17;
constexpr int COLUMN_FILE_MODIFIED_TIME = 18;
constexpr int COLUMN_FILE_CREATION_TIME = 19;
constexpr int COLUMN_REPLAYGAIN = 20;
constexpr int NUM_COLUMNS = 21;

class TrackCollectionManager;
class QMimeData;
class RecordingManager;

namespace mixxx {

class FileAccess;

} // namespace mixxx

// The BrowseTable models displays tracks
// of given directory on the HDD.
// Usage: Recording and Browse feature.
//
// TODO(XXX): Editing track metadata outside of the table view
// (e.g. in the property dialog) does not update the table view!
// Editing single fields in the table view works as expected.
class BrowseTableModel final : public QStandardItemModel, public virtual TrackModel {
    Q_OBJECT

  public:
    BrowseTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager, RecordingManager* pRec);
    virtual ~BrowseTableModel();

    // initiate table population, store path
    void setPath(mixxx::FileAccess path);

    /// Stop the BrowseThread, potentially still running and population
    /// the model, by setting an empty path in order to avoid its update
    /// signals still affecting the selection in the shared WTrackTableView
    /// before another model is loaded to it.
    void stopBrowseThread() {
        setPath({});
    };

    TrackPointer getTrack(const QModelIndex& index) const override;
    TrackPointer getTrackByRef(const TrackRef& trackRef) const override;
    TrackModel::Capabilities getCapabilities() const override;

    QString getTrackLocation(const QModelIndex& index) const override;
    TrackId getTrackId(const QModelIndex& index) const override;
    QUrl getTrackUrl(const QModelIndex& index) const final;
    CoverInfo getCoverInfo(const QModelIndex& index) const override;
    const QVector<int> getTrackRows(TrackId trackId) const override;
    void search(const QString& searchText) override;
    void removeTracks(const QModelIndexList& indices) override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    const QString currentSearch() const override;
    bool isColumnInternal(int) override;
    void moveTrack(const QModelIndex&, const QModelIndex&) override;
    void copyTracks(const QModelIndexList& indices) const override;
    bool isLocked() override { return false; }
    bool isColumnHiddenByDefault(int column) override;
    const QList<int>& searchColumns() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole) override;
    QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent) override;
    bool isColumnSortable(int column) const override;
    TrackModel::SortColumnId sortColumnIdFromColumnIndex(int index) const override;
    int columnIndexFromSortColumnId(TrackModel::SortColumnId sortColumn) const override;
    QString modelKey(bool noSearch) const override;

    bool updateTrackGenre(
            Track* pTrack,
            const QString& genre) const override;
#if defined(__EXTRA_METADATA__)
    bool updateTrackMood(
            Track* pTrack,
            const QString& mood) const override;
#endif // __EXTRA_METADATA__

    void releaseBrowseThread();

  signals:
    void saveModelState();
    void restoreModelState();

  public slots:
    void slotClear(BrowseTableModel*);
    void slotInsert(const QList<QList<QStandardItem*>>&, BrowseTableModel*);
    void trackChanged(const QString& group, TrackPointer pNewTrack, TrackPointer pOldTrack);

  private:
    TrackCollectionManager* const m_pTrackCollectionManager;

    QList<int> m_searchColumns;
    RecordingManager* m_pRecordingManager;
    BrowseThreadPointer m_pBrowseThread;
    QString m_currentDirectory;
    QString m_previewDeckGroup;
    int m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::IdMax)];
    QMap<int, TrackModel::SortColumnId> m_sortColumnIdByColumnIndex;

};
