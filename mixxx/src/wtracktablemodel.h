#ifndef WTRACKTABLEMODEL_H
#define WTRACKTABLEMODEL_H

#include <QAbstractTableModel>
#include <qcolor.h>


class TrackPlaylist;
class TrackCollection;
class DropActions;
class QColor;
class TrackInfoObject;

class WTrackTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    WTrackTableModel(QObject *parent=0);
    ~WTrackTableModel();

	void setTrackPlaylist(TrackPlaylist *pTrackPlaylist);
	void setBackgroundColor(QColor bgColor);
	void setForegroundColor(QColor fgColor);
	void setRowColor(QColor evenColor, QColor unevenColor);
	void setBpmColor(QColor confirmColor, QColor noConfirmColor);
   	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
	bool insertRow(int row, TrackInfoObject *m_pTrackInfo, const QModelIndex &parent = QModelIndex());
	TrackPlaylist *m_pTrackPlaylist;
	TrackCollection *m_pTrackCollection;

    typedef enum {
        DISABLED = -1,
        SCORE = DISABLED,
        // Note: all disabled items must be set to disabled and must appear before the first non-disabled item.
        ARTIST  ,
        TITLE   ,
        TYPE    ,
        LENGTH  ,
        BITRATE ,
        BPM     ,
        COMMENT ,
        COLUMN_COUNT // Note: COLUMN_COUNT must remain the last entry.
    } TABLE_COLUMNS;

 protected:
        Qt::DropActions supportedDropActions() const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        QMimeData *mimeData(const QModelIndexList &indexes) const;
private:

	QColor backgroundColor;
	QColor foregroundColor;
	QColor rowEvenColor;
	QColor rowUnevenColor;
	QColor bpmNoConfirmColor;
	QColor bpmConfirmColor;
	QModelIndex index;
	bool rowColors;

    //These are virtual so they can be overridden in classes that inherit
    //from this (like WPromoTracksModel)
	virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
};

#endif
