#include <qstringlist.h>

#include <QUrl>
#include <QDebug> 

#include "wtracktablemodel.h"
#include "wpromotracksmodel.h"
#include "trackcollection.h"
#include "trackinfoobject.h"
#include "trackplaylist.h"

#define MIXXX_PROMO_COL_ARTIST 0
#define MIXXX_PROMO_COL_TITLE  1
#define MIXXX_PROMO_COL_LENGTH 2
#define MIXXX_PROMO_COL_BPM    3
#define MIXXX_PROMO_COL_URL    4

WPromoTracksModel::WPromoTracksModel(QObject * parent) : WTrackTableModel(parent)
{
   //FIXME This causes terrible things, don't know why
    
    setHeaderData(MIXXX_PROMO_COL_ARTIST ,Qt::Horizontal, tr("Artist"));
    setHeaderData(MIXXX_PROMO_COL_TITLE, Qt::Horizontal, tr("Title"));
    setHeaderData(MIXXX_PROMO_COL_LENGTH, Qt::Horizontal, tr("Length"));
    setHeaderData(MIXXX_PROMO_COL_BPM, Qt::Horizontal, tr("BPM"));
    setHeaderData(MIXXX_PROMO_COL_URL, Qt::Horizontal, tr("Website"));

}

WPromoTracksModel::~WPromoTracksModel()
{
}

int WPromoTracksModel::columnCount(const QModelIndex &parent) const
{
    return 5;
}

QVariant WPromoTracksModel::data(const QModelIndex &index, int role) const
{
    TrackInfoObject *m_pTrackInfo = m_pTrackPlaylist->getTrackAt(index.row());

    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_pTrackPlaylist->getSongNum())
        return QVariant();

    if (role == Qt::DisplayRole )
    {
        switch(index.column())
        {
        case MIXXX_PROMO_COL_TITLE: return m_pTrackInfo->getTitle();
        case MIXXX_PROMO_COL_ARTIST: return m_pTrackInfo->getArtist();
        case MIXXX_PROMO_COL_LENGTH: return m_pTrackInfo->getDurationStr();
        //case COL_BITRATE: return m_pTrackInfo->getBitrateStr();
        case MIXXX_PROMO_COL_BPM: return m_pTrackInfo->getBpmStr();
        case MIXXX_PROMO_COL_URL: return m_pTrackInfo->getURL();
        //case COL_COMMENT: return m_pTrackInfo->getComment();
		default: 
			qDebug() << "index.column =" << index.column(); 
			Q_ASSERT(FALSE);	//we should never get here
			return QVariant();	
        }
    }
    else
        return QVariant();
}

QVariant WPromoTracksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch(section)
        {
        //case COL_SCORE:
        //    return QString("**");
        case MIXXX_PROMO_COL_TITLE:
            return QString("Title");
        case MIXXX_PROMO_COL_ARTIST:
            return QString("Artist");
        //case COL_TYPE:
        //    return QString("Type");
        case MIXXX_PROMO_COL_LENGTH:
            return QString("Length");
        //case MIXXX_PROMO_COL_BITRATE:
        //    return QString("kbit");
        case MIXXX_PROMO_COL_BPM:
            return QString("BPM");
        case MIXXX_PROMO_COL_URL:
            return QString("Website");
        //case COL_COMMENT:
        //    return QString("Comment");
		default:
			//this is a nasty error for the user to see, but its better than a crash and should help with debugging
            return QString("ERROR: WPromoTracksModel::headerData Invalid section parameter");	
        }
    }
    else
        return QString("%1").arg(section);

}


bool WPromoTracksModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

