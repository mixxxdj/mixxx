#ifndef BROWSETABLEMODEL_H
#define BROWSETABLEMODEL_H

#include <QStandardItemModel>
#include <QMimeData>

#include "library/trackmodel.h"
#include "library/dao/trackdao.h"
#include "library/trackcollection.h"
#include "recording/recordingmanager.h"

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

// The BrowseTable models displays tracks
// of given directory on the HDD.
// Usage: Recording and Browse feature.
class BrowseTableModel : public QStandardItemModel, public virtual TrackModel {
    Q_OBJECT

  public:
    BrowseTableModel(QObject* parent, TrackCollection* pTrackCollection, RecordingManager* pRec);
    virtual ~BrowseTableModel();
    void setPath(QString absPath);
    //reimplemented from TrackModel class
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual int getTrackId(const QModelIndex& index) const;
    TrackModel::CapabilitiesFlags getCapabilities() const;
    virtual const QLinkedList<int> getTrackRows(int trackId) const;
    virtual void search(const QString& searchText);
    virtual void removeTrack(const QModelIndex& index);
    virtual void removeTracks(const QModelIndexList& indices);
    virtual bool addTrack(const QModelIndex& index, QString location);
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
    virtual const QString currentSearch() const;
    virtual bool isColumnInternal(int);
    virtual void moveTrack(const QModelIndex&, const QModelIndex&);
    virtual QItemDelegate* delegateForColumn(const int);
    virtual bool isColumnHiddenByDefault(int column);
    virtual const QList<int>& searchColumns() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole);

  public slots:
    void slotClear(BrowseTableModel*);
    void slotInsert(const QList< QList<QStandardItem*> >&, BrowseTableModel*);

  private:
    void removeTracks(QStringList trackLocations);

    void addSearchColumn(int index);
    bool isTrackInUse(const QString& file) const;
    QList<int> m_searchColumns;
    QString m_current_path;
    TrackCollection* m_pTrackCollection;
    RecordingManager* m_pRecordingManager;
};

#endif
