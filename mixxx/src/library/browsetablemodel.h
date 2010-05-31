#ifndef BROWSETABLEMODEL_H
#define BROWSETABLEMODEL_H

#include <QFileSystemModel>
#include "trackmodel.h"
class QMimeData;

class BrowseTableModel : public QFileSystemModel, public TrackModel 
{
    Q_OBJECT
	public:
		BrowseTableModel(QObject* parent);
		~BrowseTableModel();
		virtual TrackInfoObject* getTrack(const QModelIndex& index) const;
		virtual QString getTrackLocation(const QModelIndex& index) const;
		virtual void search(const QString& searchText);
		virtual void removeTrack(const QModelIndex& index);
		virtual bool addTrack(const QModelIndex& index, QString location);
        virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
        virtual const QString currentSearch();
        virtual bool isColumnInternal(int);
        virtual void moveTrack(const QModelIndex&, const QModelIndex&);
        virtual QItemDelegate* delegateForColumn(int);
	private:
};

#endif
