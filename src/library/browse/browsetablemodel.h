#pragma once

#include <QStandardItemModel>
#include <QMimeData>

#include "library/trackmodel.h"
#include "recording/recordingmanager.h"
#include "util/file.h"
#include "library/browse/browsethread.h"

//constants
const int COLUMN_PREVIEW = 0;
const int COLUMN_FILENAME = 1;
const int COLUMN_ARTIST = 2;
const int COLUMN_TITLE = 3;
const int COLUMN_ALBUM = 4;
const int COLUMN_TRACK_NUMBER = 5;
const int COLUMN_YEAR = 6;
const int COLUMN_GENRE = 7;
const int COLUMN_COMPOSER = 8;
const int COLUMN_COMMENT = 9;
const int COLUMN_DURATION = 10;
const int COLUMN_BPM = 11;
const int COLUMN_KEY = 12;
const int COLUMN_TYPE = 13;
const int COLUMN_BITRATE = 14;
const int COLUMN_NATIVELOCATION = 15;
const int COLUMN_ALBUMARTIST = 16;
const int COLUMN_GROUPING = 17;
const int COLUMN_FILE_MODIFIED_TIME = 18;
const int COLUMN_FILE_CREATION_TIME = 19;
const int COLUMN_REPLAYGAIN = 20;

class TrackCollectionManager;

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

    void setPath(const MDir& path);

    TrackPointer getTrack(const QModelIndex& index) const override;
    TrackPointer getTrackByRef(const TrackRef& trackRef) const override;
    TrackModel::Capabilities getCapabilities() const override;

    QString getTrackLocation(const QModelIndex& index) const override;
    TrackId getTrackId(const QModelIndex& index) const override;
    CoverInfo getCoverInfo(const QModelIndex& index) const override;
    const QVector<int> getTrackRows(TrackId trackId) const override;
    void search(const QString& searchText,const QString& extraFilter = QString()) override;
    void removeTracks(const QModelIndexList& indices) override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    const QString currentSearch() const override;
    bool isColumnInternal(int) override;
    void moveTrack(const QModelIndex&, const QModelIndex&) override;
    bool isLocked() override { return false; }
    bool isColumnHiddenByDefault(int column) override;
    const QList<int>& searchColumns() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole) override;
    QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent) override;
    bool isColumnSortable(int column) const override;
    TrackModel::SortColumnId sortColumnIdFromColumnIndex(int index) const override;
    int columnIndexFromSortColumnId(TrackModel::SortColumnId sortColumn) const override;

  public slots:
    void slotClear(BrowseTableModel*);
    void slotInsert(const QList< QList<QStandardItem*> >&, BrowseTableModel*);
    void trackLoaded(const QString& group, TrackPointer pTrack);

  private:
    void addSearchColumn(int index);

    TrackCollectionManager* const m_pTrackCollectionManager;

    QList<int> m_searchColumns;
    MDir m_current_directory;
    RecordingManager* m_pRecordingManager;
    BrowseThreadPointer m_pBrowseThread;
    QString m_previewDeckGroup;
    int m_columnIndexBySortColumnId[static_cast<int>(TrackModel::SortColumnId::IdMax)];
    QMap<int, TrackModel::SortColumnId> m_sortColumnIdByColumnIndex;

};
