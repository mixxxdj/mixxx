#include <qstringlist.h>

#include <QUrl>
#include <QDebug>

#include "wtracktablemodel.h"
#include "wipodtracksmodel.h"
#include "trackcollection.h"
#include "trackinfoobject.h"
#include "trackplaylist.h"

#define MIXXX_IPOD_COL_ARTIST  0
#define MIXXX_IPOD_COL_TITLE   1
#define MIXXX_IPOD_COL_TYPE    2
#define MIXXX_IPOD_COL_LENGTH  3
#define MIXXX_IPOD_COL_BITRATE 4
#define MIXXX_IPOD_COL_BPM     5
#define MIXXX_IPOD_COL_COMMENT 6

WIPodTracksModel::WIPodTracksModel(QObject * parent) : WTrackTableModel(parent)
{
   //FIXME This causes terrible things, don't know why

    setHeaderData(MIXXX_IPOD_COL_ARTIST ,Qt::Horizontal, tr("Artist"));
    setHeaderData(MIXXX_IPOD_COL_TITLE, Qt::Horizontal, tr("Title"));
    setHeaderData(MIXXX_IPOD_COL_TYPE, Qt::Horizontal, tr("Type"));
    setHeaderData(MIXXX_IPOD_COL_LENGTH, Qt::Horizontal, tr("Length"));
    setHeaderData(MIXXX_IPOD_COL_BITRATE, Qt::Horizontal, tr("kbit"));
    setHeaderData(MIXXX_IPOD_COL_BPM, Qt::Horizontal, tr("BPM"));
    setHeaderData(MIXXX_IPOD_COL_COMMENT, Qt::Horizontal, tr("Comment"));

}

WIPodTracksModel::~WIPodTracksModel()
{
}

int WIPodTracksModel::columnCount(const QModelIndex &parent) const
{
    return 7;
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
        case MIXXX_IPOD_COL_TITLE: return m_pTrackInfo->getTitle();
        case MIXXX_IPOD_COL_ARTIST: return m_pTrackInfo->getArtist();
        case MIXXX_IPOD_COL_TYPE: return m_pTrackInfo->getType();
        case MIXXX_IPOD_COL_LENGTH: return m_pTrackInfo->getDurationStr();
        case MIXXX_IPOD_COL_BITRATE: return m_pTrackInfo->getBitrateStr();
        case MIXXX_IPOD_COL_BPM: return m_pTrackInfo->getBpmStr();
        case MIXXX_IPOD_COL_COMMENT: return m_pTrackInfo->getComment();
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
        //case COL_SCORE:
        //    return QString("**");
        case MIXXX_IPOD_COL_TITLE:
            return QString("Title");
        case MIXXX_IPOD_COL_ARTIST:
            return QString("Artist");
        case MIXXX_IPOD_COL_TYPE:
            return QString("Type");
        case MIXXX_IPOD_COL_LENGTH:
            return QString("Length");
        case MIXXX_IPOD_COL_BITRATE:
            return QString("kbit");
        case MIXXX_IPOD_COL_BPM:
            return QString("BPM");
        case MIXXX_IPOD_COL_COMMENT:
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
        case MIXXX_IPOD_COL_BPM: m_pTrackInfo->setBpm(value.toString().toFloat()); break;
        case MIXXX_IPOD_COL_COMMENT: m_pTrackInfo->setComment(value.toString()); break;
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

