#ifndef BROWSETABLEMODEL_H
#define BROWSETABLEMODEL_H

#include <QStandardItemModel>
#include <QMimeData>

#include "library/trackmodel.h"
#include "library/trackcollection.h"
#include "recording/recordingmanager.h"
#include "util/file.h"

//constants
const int COLUMN_FILENAME = 0;
const int COLUMN_ARTIST = 1;
const int COLUMN_TITLE = 2;
const int COLUMN_ALBUM = 3;
const int COLUMN_TRACK_NUMBER = 4;
const int COLUMN_YEAR = 5;
const int COLUMN_GENRE = 6;
const int COLUMN_COMPOSER = 7;
const int COLUMN_COMMENT = 8;
const int COLUMN_DURATION = 9;
const int COLUMN_BPM = 10;
const int COLUMN_KEY = 11;
const int COLUMN_TYPE = 12;
const int COLUMN_BITRATE = 13;
const int COLUMN_LOCATION = 14;
const int COLUMN_ALBUMARTIST = 15;
const int COLUMN_GROUPING = 16;
const int COLUMN_FILE_MODIFIED_TIME = 17;
const int COLUMN_FILE_CREATION_TIME = 18;

// The BrowseTable models displays tracks
// of given directory on the HDD.
// Usage: Recording and Browse feature.
class BrowseTableModel : public QStandardItemModel, public virtual TrackModel {
    Q_OBJECT

  public:
    BrowseTableModel(QObject* parent, TrackCollection* pTrackCollection, RecordingManager* pRec);
    virtual ~BrowseTableModel();
    void setPath(const MDir& path);
    //reimplemented from TrackModel class
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual TrackModel::CapabilitiesFlags getCapabilities() const;

    QString getTrackLocation(const QModelIndex& index) const;
    int getTrackId(const QModelIndex& index) const;
    const QLinkedList<int> getTrackRows(int trackId) const;
    void search(const QString& searchText,const QString& extraFilter=QString());
    void removeTrack(const QModelIndex& index);
    void removeTracks(const QModelIndexList& indices);
    bool addTrack(const QModelIndex& index, QString location);
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    const QString currentSearch() const;
    bool isColumnInternal(int);
    void moveTrack(const QModelIndex&, const QModelIndex&);
    bool isLocked() { return false;}
    bool isColumnHiddenByDefault(int column);
    const QList<int>& searchColumns() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole);
    QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent);

  public slots:
    void slotClear(BrowseTableModel*);
    void slotInsert(const QList< QList<QStandardItem*> >&, BrowseTableModel*);

  private:
    void removeTracks(QStringList trackLocations);

    void addSearchColumn(int index);
    bool isTrackInUse(const QString& file) const;
    QList<int> m_searchColumns;
    MDir m_current_directory;
    TrackCollection* m_pTrackCollection;
    RecordingManager* m_pRecordingManager;
};

#endif
