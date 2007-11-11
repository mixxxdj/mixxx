#ifndef WPLAYLISTLISTMODEL_H
#define WPLAYLISTLISTMODEL_H

#include <QAbstractTableModel>
#include <qcolor.h>
#include "trackplaylist.h"
#include "trackplaylistlist.h"

class TrackPlaylist;
class TrackCollection;
class DropActions;
class QColor;

class WPlaylistListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    WPlaylistListModel(QObject *parent=0);
    ~WPlaylistListModel();

    void setPlaylistList(TrackPlaylistList &pPlaylists);
	void setBackgroundColor(QColor bgColor);
	void setForegroundColor(QColor fgColor);
	void setRowColor(QColor evenColor, QColor unevenColor);
	void setBpmColor(QColor confirmColor, QColor noConfirmColor);
   	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
    /** List of pointers to TrackPlaylists */
    TrackPlaylistList m_qPlaylists;
	TrackCollection *m_pTrackCollection;

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


	int columnCount(const QModelIndex & parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
};

#endif
