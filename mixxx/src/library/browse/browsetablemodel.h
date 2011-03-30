#ifndef BROWSETABLEMODEL_H
#define BROWSETABLEMODEL_H

#include <QStandardItemModel>

#include "library/trackmodel.h"
#include "library/browse/browsethread.h"
#include "library/dao/trackdao.h"
#include "library/trackcollection.h"

class QMimeData;

//constants
const int COLUMN_FILENAME = 0;
const int COLUMN_ARTIST = 1;
const int COLUMN_TITLE = 2;
const int COLUMN_ALBUM = 3;
const int COLUMN_TRACK_NUMBER = 4;
const int COLUMN_YEAR = 5;
const int COLUMN_GENRE = 6;
const int COLUMN_COMMENT = 7;
const int COLUMN_DURATION = 8;
const int COLUMN_BPM = 9;
const int COLUMN_KEY = 10;
const int COLUMN_TYPE = 11;
const int COLUMN_BITRATE = 12;
const int COLUMN_LOCATION = 13;
/*
 * The BrowseTable models displays tracks
 * of given directory on the HDD.
 * Usage: Recording and Browse feature.
 */
class BrowseTableModel : public QStandardItemModel, public virtual TrackModel
{
    
    Q_OBJECT
    public:
        BrowseTableModel(QObject* parent,TrackCollection* pTrackCollection);
        virtual ~BrowseTableModel();
        void setPath(QString absPath);
        //reimplemented from TrackModel class
        virtual TrackPointer getTrack(const QModelIndex& index) const;
        virtual QString getTrackLocation(const QModelIndex& index) const;
        virtual int getTrackId(const QModelIndex& index) const;
        virtual int getTrackRow(int trackId) const;
        TrackModel::CapabilitiesFlags getCapabilities() const;

        virtual void search(const QString& searchText);
        virtual void removeTrack(const QModelIndex& index);
        virtual void removeTracks(const QModelIndexList& indices);
        virtual bool addTrack(const QModelIndex& index, QString location);
        virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
        virtual const QString currentSearch();
        virtual bool isColumnInternal(int);
        virtual void moveTrack(const QModelIndex&, const QModelIndex&);
        virtual QItemDelegate* delegateForColumn(const int);
        virtual bool isColumnHiddenByDefault(int column);
        virtual const QList<int>& searchColumns() const;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    private:


        void addSearchColumn(int index);
        QList<int> m_searchColumns;
        QString m_current_path;
        TrackCollection* m_pTrackCollection;

    public slots:
       void slotClear(BrowseTableModel*);
       void slotInsert(const QList< QList<QStandardItem*> >&, BrowseTableModel*);
};

#endif
