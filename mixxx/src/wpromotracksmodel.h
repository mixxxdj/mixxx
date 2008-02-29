#ifndef WPROMOTRACKSMODEL_H
#define WPROMOTRACKSMODEL_H

#include <QAbstractTableModel>
#include "wtracktablemodel.h"
#include <qcolor.h>

class TrackPlaylist;
class TrackCollection;
class DropActions;
class QColor;

class WPromoTracksModel : public WTrackTableModel
{
    Q_OBJECT
public:
    WPromoTracksModel(QObject *parent=0);
    ~WPromoTracksModel();

private:

    int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
};

#endif
