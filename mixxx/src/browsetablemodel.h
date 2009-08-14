#ifndef BROWSETABLEMODEL_H
#define BROWSETABLEMODEL_H

#include <QDirModel>
#include "trackmodel.h"

class BrowseTableModel : public QDirModel, public TrackModel
{
	public:
		BrowseTableModel();
		~BrowseTableModel();
		TrackInfoObject* getTrack(const QModelIndex& index) const;
		QString getTrackLocation(const QModelIndex& index) const;
		void search(const QString& searchText);
		void removeTrack(const QModelIndex& index);
		void addTrack(const QModelIndex& index, QString location);
	private:
};

#endif
