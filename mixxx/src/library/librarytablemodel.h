#ifndef LIBRARYTABLEMODEL_H
#define LIBRARYTABLEMODEL_H

#include <QtSql>
#include <QtCore>
#include "trackmodel.h"

class TrackCollection;

class LibraryTableModel : public QSqlTableModel, public virtual TrackModel
{
public:
    LibraryTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~LibraryTableModel();
    virtual TrackInfoObject* getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual void removeTrack(const QModelIndex& index);
    virtual void addTrack(const QModelIndex& index, QString location);
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QItemDelegate* delegateForColumn(const int i);
private:
    TrackCollection* m_pTrackCollection;
};

#endif
