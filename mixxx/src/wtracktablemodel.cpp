#include <qstringlist.h>

#include <QUrl>
#include <QDebug>

#include "wtracktablemodel.h"
#include "trackcollection.h"
#include "trackinfoobject.h"
#include "trackplaylist.h"

WTrackTableModel::WTrackTableModel(QObject * parent) : QAbstractTableModel(parent)
{
    rowColors = false;
    m_pTrackPlaylist = NULL;

    //setHeaderData(COL_SCORE,Qt::Horizontal, tr("**"));
    setHeaderData(COL_TITLE,Qt::Horizontal, tr("Title"));
    setHeaderData(COL_ARTIST,Qt::Horizontal, tr("Artist"));
    setHeaderData(COL_TYPE,Qt::Horizontal, tr("Type"));
    setHeaderData(COL_LENGTH,Qt::Horizontal, tr("Length"));
    setHeaderData(COL_BITRATE,Qt::Horizontal, tr("kbit"));
    setHeaderData(COL_BPM,Qt::Horizontal, tr("BPM"));
    setHeaderData(COL_COMMENT,Qt::Horizontal, tr("Comment"));
}

WTrackTableModel::~WTrackTableModel()
{
}
void WTrackTableModel::setTrackPlaylist(TrackPlaylist * pTrackPlaylist)
{
    m_pTrackPlaylist = pTrackPlaylist;
}

int WTrackTableModel::rowCount(const QModelIndex &parent) const
{
    return m_pTrackPlaylist->getSongNum();
}

int WTrackTableModel::columnCount(const QModelIndex &parent) const
{
    return 7;
}

QVariant WTrackTableModel::data(const QModelIndex &index, int role) const
{
    TrackInfoObject * m_pTrackInfo = m_pTrackPlaylist->getTrackAt(index.row());

    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_pTrackPlaylist->getSongNum())
        return QVariant();

    if (role == Qt::DisplayRole )
    {
        switch(index.column())
        {
        //case COL_SCORE: return m_pTrackInfo->getScoreStr();
        case COL_TITLE: return m_pTrackInfo->getTitle();
        case COL_ARTIST: return m_pTrackInfo->getArtist();
        case COL_TYPE: return m_pTrackInfo->getType();
        case COL_LENGTH: return m_pTrackInfo->getDurationStr();
        case COL_BITRATE: return m_pTrackInfo->getBitrateStr();
        case COL_BPM: return m_pTrackInfo->getBpmStr();
        case COL_COMMENT: return m_pTrackInfo->getComment();
		default:
			qDebug() << "index.column =" << index.column();
			Q_ASSERT(FALSE);	//we should never get here
			return QVariant();	
        }
    }

    else
        return QVariant();
}

QVariant WTrackTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch(section)
        {
        //case COL_SCORE:
        //    return QString("**");
        case COL_TITLE:
            return QString("Title");
        case COL_ARTIST:
            return QString("Artist");
        case COL_TYPE:
            return QString("Type");
        case COL_LENGTH:
            return QString("Length");
        case COL_BITRATE:
            return QString("kbit");
        case COL_BPM:
            return QString("BPM");
        case COL_COMMENT:
            return QString("Comment");
		default:
			//this is a nasty error for the user to see, but its better than a crash and should help with debugging
			return QString("ERROR: WTrackTableModel::headerData Invalid section parameter");	
        }
    }
    else
        return QString("%1").arg(section);
}

Qt::ItemFlags WTrackTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    defaultFlags |= Qt::ItemIsDragEnabled;
    switch(index.column())
    {
      case COL_BPM: return defaultFlags | QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      case COL_COMMENT: return defaultFlags | QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    return defaultFlags;
}


bool WTrackTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        TrackInfoObject * m_pTrackInfo = m_pTrackPlaylist->getTrackAt(index.row());

        switch(index.column())
        {
        case COL_BPM: m_pTrackInfo->setBpm(value.toString().toFloat()); break;
        case COL_COMMENT: m_pTrackInfo->setComment(value.toString()); break;
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool WTrackTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    TrackInfoObject * m_pTrackInfo = m_pTrackPlaylist->getTrackAt(row);
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        m_pTrackPlaylist->slotRemoveTrack(m_pTrackInfo);

    endRemoveRows();
    return true;
}

Qt::DropActions WTrackTableModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}


QMimeData *WTrackTableModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            TrackInfoObject * m_pTrackInfo = m_pTrackPlaylist->getTrackAt(index.row());
            QUrl url(QString(m_pTrackInfo->getLocation()));
            if (!url.isValid())
              qDebug() << "ERROR invalid url\n";
            else
              urls.append(url);
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
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
