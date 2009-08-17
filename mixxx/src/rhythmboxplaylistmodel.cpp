#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>

#include "rhythmboxtrackmodel.h"
#include "rhythmboxplaylistmodel.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "defs_audiofiles.h"

RhythmboxPlaylistModel::RhythmboxPlaylistModel(RhythmboxTrackModel *Rhythhmbox) :
    m_pRhythmbox(Rhythhmbox)
{
    int idx = 0;
    
    
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/playlists.xml");
    if ( ! db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/playlists.xml");
        if ( ! db.exists())
            return;
    }
    
    if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    
    QDomDocument rhythmplaylistdb;
    rhythmplaylistdb.setContent(&db);
    db.close();
    
    m_playlistNodes = rhythmplaylistdb.elementsByTagName("playlist");
    
    
    /* Filter out non song entries */
    while (( idx < m_playlistNodes.count())) {
        QDomNode n = m_playlistNodes.item(idx);
        QDomElement e = n.toElement();
        
        if (( e.isNull()) || ( e.attribute("type") != "static" )) {
            qDebug() << "Removing playlist " << e.attribute("name")
                        << "with type" << e.attribute("type");
            n.parentNode().removeChild(n);
        }
        else {
            qDebug() << "Adding Playlist" << e.attribute("name");
            QString playlist = e.attribute("name");
            
            m_mPlaylists[playlist] = n.childNodes();
            m_sCurrentPlaylist = playlist;
            
            idx++;
        }
    }
    
    
    qDebug() << rhythmplaylistdb.doctype().name();
    qDebug() << "RhythmboxPlaylistModel: m_playlistNodes size is" << m_playlistNodes.size();
    
    QMapIterator<QString, QDomNodeList>iter (m_mPlaylists);
    while (iter.hasNext())
    {
        iter.next();
        qDebug() << "RB Playlist: " << iter.key();
    }
}

RhythmboxPlaylistModel::~RhythmboxPlaylistModel()
{

}

Qt::ItemFlags RhythmboxPlaylistModel::flags ( const QModelIndex & index ) const
{
    return QAbstractTableModel::flags(index);
}

QVariant RhythmboxPlaylistModel::data ( const QModelIndex & index, int role ) const
{
    if ( m_sCurrentPlaylist == "" )
        return QVariant();
    
    if (!index.isValid())
        return QVariant();
    
    TrackInfoObject *pTrack = getTrack(index);
    if ( pTrack == NULL )
        return QVariant();
    
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case RhythmboxPlaylistModel::COLUMN_ARTIST: 
                return pTrack->getArtist();
            case RhythmboxPlaylistModel::COLUMN_TITLE:
                return pTrack->getTitle();
            case RhythmboxPlaylistModel::COLUMN_ALBUM:
                return pTrack->getAlbum();
            case RhythmboxPlaylistModel::COLUMN_DATE:
                return pTrack->getYear();
            case RhythmboxPlaylistModel::COLUMN_GENRE:
                return pTrack->getGenre();
            case RhythmboxPlaylistModel::COLUMN_LOCATION:
                return pTrack->getLocation();
            case RhythmboxPlaylistModel::COLUMN_DURATION:
                return pTrack->getDuration();
            
            
            default:
                return QVariant();
        }
    }
        
    return QVariant();
}

QVariant RhythmboxPlaylistModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    /* Only respond to requests for column header display names */
    if ( role != Qt::DisplayRole )
        return QVariant();
    
    if (orientation == Qt::Horizontal)
    {
        switch (section) 
        {
            case RhythmboxPlaylistModel::COLUMN_ARTIST:
                return QString("Artist");
            
            case RhythmboxPlaylistModel::COLUMN_TITLE:
                return QString("Title");
            
            case RhythmboxPlaylistModel::COLUMN_ALBUM:
                return QString("Album");
            
            case RhythmboxPlaylistModel::COLUMN_DATE:
                return QString("Date");
            
            case RhythmboxPlaylistModel::COLUMN_GENRE:
                return QString("Genre");
            
            case RhythmboxPlaylistModel::COLUMN_LOCATION:
                return QString("Location");
            
            case RhythmboxPlaylistModel::COLUMN_DURATION:
                return QString("Duration");
            
            default:
                return QString("Unknown");
        }
    }
    
    return QVariant();
}

int RhythmboxPlaylistModel::rowCount ( const QModelIndex & parent ) const
{
    // FIXME
    //if ( !m_mPlaylists.containts(m_sCurrentPlaylist))
    //    return 0;
    
    if ( m_sCurrentPlaylist == "" )
        return 0;
    
    return m_mPlaylists[m_sCurrentPlaylist].size();
}

int RhythmboxPlaylistModel::columnCount(const QModelIndex& parent) const
{
    if (parent != QModelIndex()) //Some weird thing for table-based models.
        return 0;
    return RhythmboxPlaylistModel::NUM_COLUMNS;
}

void RhythmboxPlaylistModel::addTrack(const QModelIndex& index, QString location)
{
    //Should do nothing... hmmm
}

/** Removes a track from the library track collection. */
void RhythmboxPlaylistModel::removeTrack(const QModelIndex& index)
{
    //Should do nothing... hmmm
}

QString RhythmboxPlaylistModel::getTrackLocation(const QModelIndex& index) const
{
    //FIXME
    return QString();
}

TrackInfoObject * RhythmboxPlaylistModel::getTrack(const QModelIndex& index) const
{
    QDomNodeList playlistTrackList = m_mPlaylists[m_sCurrentPlaylist];
    QDomNode pnode = playlistTrackList.at(index.row());
    QString location = pnode.toElement().text();
    
 	return m_pRhythmbox->getTrackByLocation(location);
}

QList<QString> RhythmboxPlaylistModel::getPlaylists()
{
    return m_mPlaylists.keys();
}

void RhythmboxPlaylistModel::setPlaylist(QString playlist)
{
    if ( m_mPlaylists.contains(playlist))
        m_sCurrentPlaylist = playlist;
    else
        m_sCurrentPlaylist = "";
    
    // force the layout to update
    emit(layoutChanged());
}

void RhythmboxPlaylistModel::search(const QString& searchText)
{

}
