#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>

#include "rhythmboxtrackmodel.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "defs_audiofiles.h"

RhythmboxTrackModel::RhythmboxTrackModel()
{
    int idx = 0;
    
    
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/rhythmdb.xml");
    if ( ! db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/rhythmdb.xml");
        if ( ! db.exists())
            return;
    }
    
    if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    
    QDomDocument rhythmdb;
    rhythmdb.setContent(&db);
    db.close();
    
    m_entryNodes = rhythmdb.elementsByTagName("entry");
    
    
    /* Filter out non song entries */
    while (( idx < m_entryNodes.count()) && (m_entryNodes.count() > 0)) {
        QDomNode n = m_entryNodes.item(idx);
        QDomElement e = n.toElement();
        
        if (( e.isNull()) || ( e.attribute("type") != "song" ))
            n.parentNode().removeChild(n);
        else {
            m_mTracksByLocation[n.firstChildElement("location").text()] = n;
            idx++;
        }
    }
    
    qDebug() << rhythmdb.doctype().name();
    qDebug() << "RhythmboxTrackModel: m_entryNodes size is" << m_entryNodes.size();
}

RhythmboxTrackModel::~RhythmboxTrackModel()
{

}

Qt::ItemFlags RhythmboxTrackModel::flags ( const QModelIndex & index ) const
{
    return QAbstractTableModel::flags(index);
}

QVariant RhythmboxTrackModel::data ( const QModelIndex & index, int role ) const
{
    if (!index.isValid())
        return QVariant();
    
    if (m_entryNodes.size() < index.row())
        return QVariant();
    
    QDomNode songNode = m_entryNodes.at(index.row());
    QDomElement e = songNode.toElement();
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case RhythmboxTrackModel::COLUMN_ARTIST: 
                return songNode.firstChildElement("artist").text();
            case RhythmboxTrackModel::COLUMN_TITLE:
                return songNode.firstChildElement("title").text();
            case RhythmboxTrackModel::COLUMN_ALBUM:
                return songNode.firstChildElement("album").text();
            case RhythmboxTrackModel::COLUMN_DATE:
                return songNode.firstChildElement("date").text();
            case RhythmboxTrackModel::COLUMN_GENRE:
                return songNode.firstChildElement("genre").text();
            case RhythmboxTrackModel::COLUMN_LOCATION:
                return songNode.firstChildElement("location").text();
            case RhythmboxTrackModel::COLUMN_DURATION:
                return songNode.firstChildElement("duration").text();
            
            
            default:
                return QVariant();
        }
    }
        
    return QVariant();
}

QVariant RhythmboxTrackModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    /* Only respond to requests for column header display names */
    if ( role != Qt::DisplayRole )
        return QVariant();
    
    if (orientation == Qt::Horizontal)
    {
        switch (section) 
        {
            case RhythmboxTrackModel::COLUMN_ARTIST:
                return QString("Artist");
            
            case RhythmboxTrackModel::COLUMN_TITLE:
                return QString("Title");
            
            case RhythmboxTrackModel::COLUMN_ALBUM:
                return QString("Album");
            
            case RhythmboxTrackModel::COLUMN_DATE:
                return QString("Date");
            
            case RhythmboxTrackModel::COLUMN_GENRE:
                return QString("Genre");
            
            case RhythmboxTrackModel::COLUMN_LOCATION:
                return QString("Location");
            
            case RhythmboxTrackModel::COLUMN_DURATION:
                return QString("Duration");
            
            default:
                return QString("Unknown");
        }
    }
    
    return QVariant();
}

int RhythmboxTrackModel::rowCount ( const QModelIndex & parent ) const
{
    return m_entryNodes.size();
}

int RhythmboxTrackModel::columnCount(const QModelIndex& parent) const
{
    if (parent != QModelIndex()) //Some weird thing for table-based models.
        return 0;
    return RhythmboxTrackModel::NUM_COLUMNS;
}

void RhythmboxTrackModel::addTrack(const QModelIndex& index, QString location)
{
    //Should do nothing... hmmm
}

/** Removes a track from the library track collection. */
void RhythmboxTrackModel::removeTrack(const QModelIndex& index)
{
    //Should do nothing... hmmm
}

QString RhythmboxTrackModel::getTrackLocation(const QModelIndex& index) const
{
    //FIXME
    return QString();
}
  
TrackInfoObject * RhythmboxTrackModel::getTrack(const QModelIndex& index) const
{
    //FIXME
    qDebug() << "STUB: RhythmboxTrackModel::getTrack()";
 	TrackInfoObject* track = new TrackInfoObject();
 	
    QDomNode songNode = m_entryNodes.at(index.row());
 	track->setLocation(QUrl(songNode.firstChildElement("location").text()).toLocalFile());
 	track->setArtist(songNode.firstChildElement("artist").text());
 	track->setTitle(songNode.firstChildElement("title").text());
 	track->setDuration(songNode.firstChildElement("duration").text().toUInt()); 	 	    
    return track;
}

TrackInfoObject * RhythmboxTrackModel::getTrackByLocation(const QString& location) const
{
    TrackInfoObject *track = new TrackInfoObject();
    
    if ( !m_mTracksByLocation.contains(location))
        return NULL;
    
    QDomNode songNode = m_mTracksByLocation[location];
    track->setLocation(QUrl(songNode.firstChildElement("location").text()).toLocalFile());
 	track->setArtist(songNode.firstChildElement("artist").text());
 	track->setTitle(songNode.firstChildElement("title").text());
 	track->setDuration(songNode.firstChildElement("duration").text().toUInt()); 	 	    
    return track;
}

void RhythmboxTrackModel::search(const QString& searchText)
{
    //FIXME
}
