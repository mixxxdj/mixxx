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
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/rhythmdb.xml");
    if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    
    QDomDocument rhythmdb;
    rhythmdb.setContent(&db);
    db.close();
    
    m_entryNodes = rhythmdb.elementsByTagName("entry");
    //m_entryNodes = rhythmdb.documentElement().childNodes();
        
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
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case RhythmboxTrackModel::COLUMN_ARTIST: 
                return songNode.firstChildElement("artist").text();
            case RhythmboxTrackModel::COLUMN_TITLE:
                return songNode.firstChildElement("title").text(); 
            
            
            default:
                return QVariant();
        }
    }
        
    return QVariant();
}

QVariant RhythmboxTrackModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if (orientation == Qt::Horizontal)
    {
        switch (section) 
        {
            case RhythmboxTrackModel::COLUMN_ARTIST:
                return QVariant("Artist");
                break;
            
            case RhythmboxTrackModel::COLUMN_TITLE:
                return QVariant("Title");
                break;
            
            default: return "Unknown";
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

void RhythmboxTrackModel::search(const QString& searchText)
{

}

	
