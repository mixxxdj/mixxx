
#include <QtCore>
#include "browsetablemodel.h"


BrowseTableModel::BrowseTableModel() : QDirModel()
{
	
}

BrowseTableModel::~BrowseTableModel()
{
	
}

TrackInfoObject* BrowseTableModel::getTrack(const QModelIndex& index) const
{
	//TODO

    return NULL;
}

QString BrowseTableModel::getTrackLocation(const QModelIndex& index) const
{
	//TODO
    return QString("");
}

void BrowseTableModel::search(const QString& searchText)
{
	//TODO
}

void BrowseTableModel::removeTrack(const QModelIndex& index)
{
	//TODO
}

void BrowseTableModel::addTrack(const QModelIndex& index, QString location)
{
	//TODO
}
