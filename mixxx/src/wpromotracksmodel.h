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

    typedef enum {
        DISABLED = -1,
        SCORE = DISABLED,
        // Note: all disabled items must be set to disabled and must appear before the first non-disabled item.
        ARTIST  ,
        TITLE   ,
        LENGTH  ,
        BPM     ,
        URL     ,
        COLUMN_COUNT // Note: COLUMN_COUNT must remain the last entry.
    } TABLE_COLUMNS;

  private:

    int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
//	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
};

#endif
