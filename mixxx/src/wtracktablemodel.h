#ifndef WTRACKTABLEMODEL_H
#define WTRACKTABLEMODEL_H

#include <QAbstractTableModel>
#include <qcolor.h>

class TrackPlaylist;
class TrackCollection;
class QColor;

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
	TrackPlaylist *m_pTrackPlaylist;
	TrackCollection *m_pTrackCollection;

private:
	
	QColor backgroundColor;
	QColor foregroundColor;
	QColor rowEvenColor;
	QColor rowUnevenColor;
	QColor bpmNoConfirmColor;
	QColor bpmConfirmColor;
	QModelIndex index;
	bool rowColors;

	
	int columnCount(const QModelIndex & parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
};

#endif
