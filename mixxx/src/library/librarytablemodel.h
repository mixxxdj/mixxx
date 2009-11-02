#ifndef LIBRARYTABLEMODEL_H
#define LIBRARYTABLEMODEL_H

#include <QtSql>
#include <QtCore>
#include "library/trackmodel.h"
#include "library/dao/trackdao.h"

class TrackCollection;

class LibraryTableModel : public QSqlRelationalTableModel, public virtual TrackModel
{
public:
    LibraryTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~LibraryTableModel();
    virtual TrackInfoObject* getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual void search(const QString& searchText);
    virtual const QString currentSearch();
    virtual bool isColumnInternal(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual void addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex);
    virtual QVariant data(const QModelIndex& item, int role) const;


    QMimeData* mimeData(const QModelIndexList &indexes) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QItemDelegate* delegateForColumn(const int i);
    TrackModel::CapabilitiesFlags getCapabilities() const;
private:
    //TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDao;
    QString m_currentSearch;
};

#endif
