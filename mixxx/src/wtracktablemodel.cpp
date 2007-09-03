#include <qstringlist.h>


#include "wtracktablemodel.h"
#include "trackcollection.h"
#include "trackinfoobject.h"
#include "trackplaylist.h"

WTrackTableModel::WTrackTableModel(QObject *parent) : QAbstractTableModel(parent)
{
	rowColors = false;
	setHeaderData(0,Qt::Horizontal, tr("**"));
	setHeaderData(1,Qt::Horizontal, tr("Title"));
	setHeaderData(2,Qt::Horizontal, tr("Artist"));
	setHeaderData(3,Qt::Horizontal, tr("Type"));
	setHeaderData(4,Qt::Horizontal, tr("Duration"));
	setHeaderData(5,Qt::Horizontal, tr("Bitrate"));
	setHeaderData(6,Qt::Horizontal, tr("BPM"));
	setHeaderData(7,Qt::Horizontal, tr("Comment"));
}
WTrackTableModel::~WTrackTableModel()
{
}
void WTrackTableModel :: setTrackPlaylist(TrackPlaylist *pTrackPlaylist)
{
	m_pTrackPlaylist = pTrackPlaylist;
}
int WTrackTableModel :: rowCount(const QModelIndex &parent) const
{
	return m_pTrackPlaylist->getSongNum();
}
int WTrackTableModel :: columnCount(const QModelIndex &parent) const
{
	return 8;
}
QVariant WTrackTableModel :: data(const QModelIndex &index, int role) const
{   
	TrackInfoObject *m_pTrackInfo = m_pTrackPlaylist->getTrackAt(index.row());

	if (!index.isValid())
		return QVariant();

	if (index.row() >= m_pTrackPlaylist->getSongNum())
        return QVariant();

	if (role == Qt::ForegroundRole )
	{
		return foregroundColor;
	}
	else if (role == Qt::DisplayRole )
	{
		switch(index.column())
		{
		case 0: return m_pTrackInfo->getScoreStr();
		case 1: return m_pTrackInfo->getTitle();
		case 2: return m_pTrackInfo->getArtist();
		case 3: return m_pTrackInfo->getType();
		case 4: return m_pTrackInfo->getDurationStr();
		case 5: return m_pTrackInfo->getBitrateStr();
		case 6: return m_pTrackInfo->getBpmStr();
		case 7: return m_pTrackInfo->getComment();
		}
	}
	
    else
        return QVariant();
}
QVariant WTrackTableModel :: headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

    if (orientation == Qt::Horizontal)
	{
		switch(section)
		{
		case 0:
			return QString("**");
		case 1:
			return QString("Title");
		case 2:
			return QString("Artist");
		case 3:
			return QString("Type");
		case 4:
			return QString("Duration");
		case 5:
			return QString("Bitrate");
		case 6:
			return QString("BPM");
		case 7:
			return QString("Comment");
		}
	}
	else
		return QString("%1").arg(section);
}
Qt::ItemFlags WTrackTableModel::flags(const QModelIndex &index) const
{
        if (!index.isValid())
            return Qt::ItemIsEnabled;
		switch(index.column())
		{
		case 6: return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
		case 7: return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
		}
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
bool WTrackTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (index.isValid() && role == Qt::EditRole) 
		{
			TrackInfoObject *m_pTrackInfo = m_pTrackPlaylist->getTrackAt(index.row());
		
            switch(index.column())
			{
			case 6: m_pTrackInfo->setBpm(value.toString().toFloat()); break;
			case 7: m_pTrackInfo->setComment(value.toString()); break;
			}
			emit dataChanged(index, index);
            return true;
        }
        return false;
    }
bool WTrackTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
	Q_UNUSED(parent);
	TrackInfoObject *m_pTrackInfo = m_pTrackPlaylist->getTrackAt(row);
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
		return false;
    
	beginRemoveRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
		m_pTrackPlaylist->slotRemoveTrack(m_pTrackInfo);
	
	endRemoveRows();
	return true;
} 
void WTrackTableModel::setBackgroundColor(QColor bgColor)
{
	backgroundColor = bgColor;
}
void WTrackTableModel::setForegroundColor(QColor fgColor)
{
	foregroundColor = fgColor;
}
void WTrackTableModel::setRowColor(QColor evenColor, QColor unevenColor)
{
	backgroundColor = evenColor;
	rowUnevenColor = unevenColor;
}
void WTrackTableModel::setBpmColor(QColor confirmColor, QColor noConfirmColor)
{
	bpmNoConfirmColor = noConfirmColor;
	bpmConfirmColor = confirmColor;
	rowColors = true;
}
