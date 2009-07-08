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

    //setHeaderData(WTrackTableModel::SCORE,Qt::Horizontal, tr("**"));
    setHeaderData(WTrackTableModel::TITLE,Qt::Horizontal, tr("Title"));
    setHeaderData(WTrackTableModel::ARTIST,Qt::Horizontal, tr("Artist"));
    setHeaderData(WTrackTableModel::TYPE,Qt::Horizontal, tr("Type"));
    setHeaderData(WTrackTableModel::LENGTH,Qt::Horizontal, tr("Length"));
    setHeaderData(WTrackTableModel::BITRATE,Qt::Horizontal, tr("kbit"));
    setHeaderData(WTrackTableModel::BPM,Qt::Horizontal, tr("BPM"));
    setHeaderData(WTrackTableModel::COMMENT,Qt::Horizontal, tr("Comment"));
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
    return WTrackTableModel::COLUMN_COUNT;
}

QVariant WTrackTableModel::data(const QModelIndex &index, int role) const
{
    TrackInfoObject * m_pTrackInfo = m_pTrackPlaylist->at(index.row());

    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_pTrackPlaylist->getSongNum())
        return QVariant();

    if (role == Qt::DisplayRole )
    {
        switch(index.column())
        {
        //case WTrackTableModel::SCORE: return m_pTrackInfo->getScoreStr();
        case WTrackTableModel::TITLE: return m_pTrackInfo->getTitle();
        case WTrackTableModel::ARTIST: return m_pTrackInfo->getArtist();
        case WTrackTableModel::TYPE: return m_pTrackInfo->getType();
        case WTrackTableModel::LENGTH: return m_pTrackInfo->getDurationStr();
        case WTrackTableModel::BITRATE: return m_pTrackInfo->getBitrateStr();
        case WTrackTableModel::BPM: return m_pTrackInfo->getBpmStr();
        case WTrackTableModel::COMMENT: return m_pTrackInfo->getComment();
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
        //case WTrackTableModel::SCORE:
        //    return QString("**");
        case WTrackTableModel::TITLE:
            return QString("Title");
        case WTrackTableModel::ARTIST:
            return QString("Artist");
        case WTrackTableModel::TYPE:
            return QString("Type");
        case WTrackTableModel::LENGTH:
            return QString("Length");
        case WTrackTableModel::BITRATE:
            return QString("kbit");
        case WTrackTableModel::BPM:
            return QString("BPM");
        case WTrackTableModel::COMMENT:
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
      case WTrackTableModel::BPM: return defaultFlags | QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      case WTrackTableModel::COMMENT: return defaultFlags | QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    return defaultFlags;
}


bool WTrackTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        TrackInfoObject * m_pTrackInfo = m_pTrackPlaylist->at(index.row());

        switch(index.column())
        {
        case WTrackTableModel::BPM: m_pTrackInfo->setBpm(value.toString().toFloat()); break;
        case WTrackTableModel::COMMENT: m_pTrackInfo->setComment(value.toString()); break;
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool WTrackTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    //TrackInfoObject * pTrackInfo = m_pTrackPlaylist->at(row);

    //Check bounds
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        m_pTrackPlaylist->removeAt(row);

    endRemoveRows();
    return true;
}

bool WTrackTableModel::insertRow(int row, TrackInfoObject *pTrackInfo, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    //Check bounds
    if (row < 0 || row > rowCount(parent))
        row = rowCount(parent);

    beginInsertRows(QModelIndex(), row, row);

    m_pTrackPlaylist->insert(row, pTrackInfo);

    endInsertRows();
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
            TrackInfoObject * m_pTrackInfo = m_pTrackPlaylist->at(index.row());
			QUrl url = QUrl::fromLocalFile(QString(m_pTrackInfo->getLocation()));

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
