#ifndef BROWSETABLEMODEL_H
#define BROWSETABLEMODEL_H

#include <QStandardItemModel>
#include "trackmodel.h"
#include "browsethread.h"

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

class BrowseTableModel : public QStandardItemModel, public TrackModel
{
    
    Q_OBJECT
    public:
        BrowseTableModel(QObject* parent);
        ~BrowseTableModel();

        void setPath(QString absPath);
        //reimplemented from TrackModel class
        virtual TrackPointer getTrack(const QModelIndex& index) const;
        virtual QString getTrackLocation(const QModelIndex& index) const;
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
    private:
       void addSearchColumn(int index);

        BrowseThread m_backgroundThread;
        QList<int> m_searchColumns;
    public slots:
       void slotClear();
       void slotInsert(const QList< QList<QStandardItem*> >&);
};

#endif
