#include <qstringlist.h>

#include <QUrl>
#include <QDebug>

#include "wtracktablemodel.h"
#include "wipodtracksmodel.h"
#include "trackcollection.h"
#include "trackinfoobject.h"
#include "trackplaylist.h"

WIPodTracksModel::WIPodTracksModel(QObject * parent) : WTrackTableModel(parent)
{
    setHeaderData(WIPodTracksModel::ARTIST ,Qt::Horizontal, tr("Artist"));
    setHeaderData(WIPodTracksModel::TITLE, Qt::Horizontal, tr("Title"));
    setHeaderData(WIPodTracksModel::TYPE, Qt::Horizontal, tr("Type"));
    setHeaderData(WIPodTracksModel::LENGTH, Qt::Horizontal, tr("Length"));
    setHeaderData(WIPodTracksModel::BITRATE, Qt::Horizontal, tr("kbit"));
    setHeaderData(WIPodTracksModel::BPM, Qt::Horizontal, tr("BPM"));
    setHeaderData(WIPodTracksModel::COMMENT, Qt::Horizontal, tr("Comment"));

}

WIPodTracksModel::~WIPodTracksModel()
{
}

int WIPodTracksModel::columnCount(const QModelIndex &parent) const
{
    return WIPodTracksModel::COLUMN_COUNT;
}

QVariant WIPodTracksModel::data(const QModelIndex &index, int role) const
{
    TrackInfoObject *m_pTrackInfo = m_pTrackPlaylist->at(index.row());

    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_pTrackPlaylist->getSongNum())
        return QVariant();

    if (role == Qt::DisplayRole )
    {
        switch(index.column())
        {
        case WIPodTracksModel::TITLE: return m_pTrackInfo->getTitle();
        case WIPodTracksModel::ARTIST: return m_pTrackInfo->getArtist();
        case WIPodTracksModel::TYPE: return m_pTrackInfo->getType();
        case WIPodTracksModel::LENGTH: return m_pTrackInfo->getDurationStr();
        case WIPodTracksModel::BITRATE: return m_pTrackInfo->getBitrateStr();
        case WIPodTracksModel::BPM: return m_pTrackInfo->getBpmStr();
        case WIPodTracksModel::COMMENT: return m_pTrackInfo->getComment();
	default:
          qDebug() << "index.column =" << index.column();
          Q_ASSERT(FALSE);    //we should never get here
          return QVariant();
        }
    }
    else
        return QVariant();
}

QVariant WIPodTracksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch(section)
        {
//        case WIPodTracksModel::SCORE:
        //    return QString("**");
        case WIPodTracksModel::TITLE:
            return QString("Title");
        case WIPodTracksModel::ARTIST:
            return QString("Artist");
        case WIPodTracksModel::TYPE:
            return QString("Type");
        case WIPodTracksModel::LENGTH:
            return QString("Length");
        case WIPodTracksModel::BITRATE:
            return QString("kbit");
        case WIPodTracksModel::BPM:
            return QString("BPM");
        case WIPodTracksModel::COMMENT:
            return QString("Comment");
        default:
	//this is a nasty error for the user to see, but its better than a crash and should help with debugging
            return QString("ERROR: WIPodTracksModel::headerData Invalid section parameter");
        }
    }
    else
        return QString("%1").arg(section);

}


bool WIPodTracksModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        TrackInfoObject * m_pTrackInfo = m_pTrackPlaylist->at(index.row());

        switch(index.column())
        {
        case WIPodTracksModel::BPM: m_pTrackInfo->setBpm(value.toString().toFloat()); break;
        case WIPodTracksModel::COMMENT: m_pTrackInfo->setComment(value.toString()); break;
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

