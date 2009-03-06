#include <qstringlist.h>

#include <QUrl>
#include <QDebug>

#include "wtracktablemodel.h"
#include "wpromotracksmodel.h"
#include "trackcollection.h"
#include "trackinfoobject.h"
#include "trackplaylist.h"

WPromoTracksModel::WPromoTracksModel(QObject * parent) : WTrackTableModel(parent)
{
    setHeaderData(WPromoTracksModel::ARTIST ,Qt::Horizontal, tr("Artist"));
    setHeaderData(WPromoTracksModel::TITLE, Qt::Horizontal, tr("Title"));
    setHeaderData(WPromoTracksModel::LENGTH, Qt::Horizontal, tr("Length"));
    setHeaderData(WPromoTracksModel::BPM, Qt::Horizontal, tr("BPM"));
    setHeaderData(WPromoTracksModel::URL, Qt::Horizontal, tr("Website"));
}

WPromoTracksModel::~WPromoTracksModel()
{
}

int WPromoTracksModel::columnCount(const QModelIndex &parent) const
{
    return WPromoTracksModel::COLUMN_COUNT;
}

QVariant WPromoTracksModel::data(const QModelIndex &index, int role) const
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
        case WPromoTracksModel::TITLE: return m_pTrackInfo->getTitle();
        case WPromoTracksModel::ARTIST: return m_pTrackInfo->getArtist();
        case WPromoTracksModel::LENGTH: return m_pTrackInfo->getDurationStr();
        //case WPromoTracksModel::BITRATE: return m_pTrackInfo->getBitrateStr();
        case WPromoTracksModel::BPM: return m_pTrackInfo->getBpmStr();
        case WPromoTracksModel::URL: return m_pTrackInfo->getURL();
        //case WPromoTracksModel::COMMENT: return m_pTrackInfo->getComment();
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
        case WPromoTracksModel::TITLE:
            return QString("Title");
        case WPromoTracksModel::ARTIST:
            return QString("Artist");
        case WPromoTracksModel::LENGTH:
            return QString("Length");
        case WPromoTracksModel::BPM:
            return QString("BPM");
        case WPromoTracksModel::URL:
            return QString("Website");
		default:
			//this is a nasty error for the user to see, but its better than a crash and should help with debugging
            return QString("ERROR: WPromoTracksModel::headerData Invalid section parameter");
        }
    }
    else
        return QString("%1").arg(section);

}

// Don't know if this is needed or not...  But  it looks like cut-and-paste from elsewhere
//bool WPromoTracksModel::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//    if (index.isValid() && role == Qt::EditRole)
//    {
//        TrackInfoObject * m_pTrackInfo = m_pTrackPlaylist->at(index.row());
//
//        switch(index.column())
//        {
//        case COL_BPM: m_pTrackInfo->setBpm(value.toString().toFloat()); break;
//        case COL_COMMENT: m_pTrackInfo->setComment(value.toString()); break;
//        }
//        emit dataChanged(index, index);
//        return true;
//    }
//
//    return false;
//}
//
