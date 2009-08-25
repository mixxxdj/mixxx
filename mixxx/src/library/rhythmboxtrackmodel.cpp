#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>
#include <QtXmlPatterns/QXmlQuery>

#include "durationdelegate.h"
#include "rhythmboxtrackmodel.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "defs_audiofiles.h"

RhythmboxTrackModel::RhythmboxTrackModel()
{
    QXmlQuery query;
    QString res;
    QDomDocument rhythmdb;
    
    
    /*
     * Try and open the Rhythmbox DB. An API call which tells us where
     * the file is would be nice.
     */
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/rhythmdb.xml");
    if ( ! db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/rhythmdb.xml");
        if ( ! db.exists())
            return;
    }
    
    if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    
    /*
     * Use QXmlQuery to execute an XPath query. We add the version to
     * the XPath query to make sure it is the schema we expect.
     */
    query.setFocus(&db);
    query.setQuery("rhythmdb[@version='1.6']/entry[@type='song']");
    if ( ! query.isValid())
        return;
    
    query.evaluateTo(&res);
    db.close();
    
    
    /*
     * Parse the result as an XML file. These shennanigans actually
     * reduce the load time from a minute to a matter of seconds.
     */
    rhythmdb.setContent("<rhythmdb version='1.6'>" + res + "</rhythmdb>");
    m_trackNodes = rhythmdb.elementsByTagName("entry");
    
    
    for (int i = 0; i < m_trackNodes.count(); i++) {
        QDomNode n = m_trackNodes.at(i);
        QString location = n.firstChildElement("location").text();
        
        m_mTracksByLocation[location] = n;
    }
    
    qDebug() << rhythmdb.doctype().name();
    qDebug() << "RhythmboxTrackModel: m_entryNodes size is" << m_trackNodes.size();
    
    
    addColumnName(RhythmboxTrackModel::COLUMN_ARTIST, "Artist");
    addColumnName(RhythmboxTrackModel::COLUMN_TITLE, "Title");
    addColumnName(RhythmboxTrackModel::COLUMN_ALBUM, "Album");
    addColumnName(RhythmboxTrackModel::COLUMN_DATE, "Date");
    addColumnName(RhythmboxTrackModel::COLUMN_GENRE, "Genre");
    addColumnName(RhythmboxTrackModel::COLUMN_LOCATION, "Location");
    addColumnName(RhythmboxTrackModel::COLUMN_DURATION, "Duration");
    
}

RhythmboxTrackModel::~RhythmboxTrackModel()
{

}

QItemDelegate* RhythmboxTrackModel::delegateForColumn(const int i) {
    if (i == RhythmboxTrackModel::COLUMN_DURATION) {
        return new DurationDelegate();
    }
    return NULL;
}

QVariant RhythmboxTrackModel::getTrackColumnData(QDomNode songNode, const QModelIndex& index) const
{
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

TrackInfoObject *RhythmboxTrackModel::parseTrackNode(QDomNode songNode) const
{
    TrackInfoObject *pTrack = new TrackInfoObject();
    
    
    pTrack->setArtist(songNode.firstChildElement("artist").text());
    pTrack->setTitle(songNode.firstChildElement("title").text());
    pTrack->setAlbum(songNode.firstChildElement("album").text());
    pTrack->setYear(songNode.firstChildElement("date").text());
    pTrack->setGenre(songNode.firstChildElement("genre").text());
    pTrack->setDuration(songNode.firstChildElement("duration").text().toUInt());
    pTrack->setLocation(QUrl(songNode.firstChildElement("location").text()).toLocalFile());
    
    return pTrack;
}
