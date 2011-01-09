#include <QMessageBox>
#include <QtDebug>
#include <QStringList>

#include "library/rhythmboxtrackmodel.h"
#include "library/rhythmboxplaylistmodel.h"
#include "library/rhythmboxfeature.h"
#include "treeitem.h"

RhythmboxFeature::RhythmboxFeature(QObject* parent, TrackCollection* pTrackCollection)
    : LibraryFeature(parent),
      m_pTrackCollection(pTrackCollection),
      m_database(m_pTrackCollection->getDatabase()) {

    m_pRhythmboxTrackModel = new RhythmboxTrackModel(this, m_pTrackCollection);
    m_pRhythmboxPlaylistModel = new RhythmboxPlaylistModel(this, m_pTrackCollection);
    m_isActivated =  false;
}

RhythmboxFeature::~RhythmboxFeature() {
    delete m_pRhythmboxTrackModel;
    delete m_pRhythmboxPlaylistModel;
}

bool RhythmboxFeature::isSupported() {
    return (QFile::exists(QDir::homePath() + "/.gnome2/rhythmbox/rhythmdb.xml") ||
            QFile::exists(QDir::homePath() + "/.local/share/rhythmbox/rhythmdb.xml"));
}

QVariant RhythmboxFeature::title() {
    return tr("Rhythmbox");
}

QIcon RhythmboxFeature::getIcon() {
    return QIcon(":/images/library/ic_library_rhythmbox.png");
}

TreeItemModel* RhythmboxFeature::getChildModel() {
    return &m_childModel;
}

void RhythmboxFeature::activate() {
     qDebug() << "RhythmboxFeature::activate()";
    
    if(!m_isActivated){
        if (QMessageBox::question(
            NULL,
            tr("Load Rhythmbox Library?"),
            tr("Would you like to load your Rhythmbox library?"),
            QMessageBox::Ok,
            QMessageBox::Cancel)
            == QMessageBox::Cancel) {
            return;
        }
        if(importMusicCollection() && importPlaylists())
            m_isActivated =  true;
    }
    emit(showTrackModel(m_pRhythmboxTrackModel));
}

void RhythmboxFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "RhythmboxFeature::activateChild()" << index;
    QString playlist = index.data().toString();
    qDebug() << "Activating " << playlist;
    m_pRhythmboxPlaylistModel->setPlaylist(playlist);
    emit(showTrackModel(m_pRhythmboxPlaylistModel));
}

void RhythmboxFeature::onRightClick(const QPoint& globalPos) {
}

void RhythmboxFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
}

bool RhythmboxFeature::dropAccept(QUrl url) {
    return false;
}

bool RhythmboxFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool RhythmboxFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool RhythmboxFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}
bool RhythmboxFeature::importMusicCollection()
{
    /*
     * Try and open the Rhythmbox DB. An API call which tells us where
     * the file is would be nice.
     */
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/rhythmdb.xml");
    if ( ! db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/rhythmdb.xml");
        if ( ! db.exists())
            return false;
    }

    if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    //Delete all table entries of Traktor feature
    m_database.transaction();
    clearTable("rhythmbox_playlist_tracks");
    clearTable("rhythmbox_library");
    clearTable("rhythmbox_playlists");
    m_database.commit();
    
    m_database.transaction();
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO rhythmbox_library (artist, title, album, year, genre, comment, tracknumber,"
                  "bpm, bitrate,"
                  "duration, location,"
                  "rating ) "
                  "VALUES (:artist, :title, :album, :year, :genre, :comment, :tracknumber,"
                  ":bpm, :bitrate,"
                  ":duration, :location, :rating )");
    
    
    QXmlStreamReader xml(&db);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "entry") {
            QXmlStreamAttributes attr = xml.attributes();
            //Check if we really parse a track and not album art information
            if(attr.value("type").toString() == "song"){
                importTrack(xml, query);
                
            }
        }
    }
    m_database.commit();
    m_pRhythmboxTrackModel->select();

    if (xml.hasError()) {
        // do error handling
        qDebug() << "Cannot process Rhythmbox music collection";
        qDebug() << "XML ERROR: " << xml.errorString();
        return false;
    }
    
    
    db.close();
    return true;

}
bool RhythmboxFeature::importPlaylists()
{
    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/playlists.xml");
    if ( ! db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/playlists.xml");
        if (!db.exists())
            return false;
    }
    //Open file
     if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    
    QSqlQuery query_insert_to_playlists(m_database);
    query_insert_to_playlists.prepare("INSERT INTO rhythmbox_playlists (id, name) "
                                      "VALUES (:id, :name)");

    QSqlQuery query_insert_to_playlist_tracks(m_database);
    query_insert_to_playlist_tracks.prepare("INSERT INTO rhythmbox_playlist_tracks (playlist_id, track_id) "
                                            "VALUES (:playlist_id, :track_id)");
    //The tree structure holding the playlists
    TreeItem *rootItem = new TreeItem("$root","$root", this);
                                            
    QXmlStreamReader xml(&db);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == "playlist") {
            QXmlStreamAttributes attr = xml.attributes();
            
            //Only parse non build-in playlists
            if(attr.value("type").toString() == "static"){
                QString playlist_name = attr.value("name").toString();
                
                //Construct the childmodel
                TreeItem * item = new TreeItem(playlist_name,playlist_name, this, rootItem);
                rootItem->appendChild(item);
                
                //Execute SQL statement
                query_insert_to_playlists.bindValue(":name", playlist_name);
                
                bool success = query_insert_to_playlists.exec();
                if(!success){
                    qDebug() << "SQL Error in RhythmboxFeature.cpp: line" << __LINE__ << " " 
                                    << query_insert_to_playlists.lastError();
                    return false;
                }
                //get playlist_id
                int playlist_id = query_insert_to_playlists.lastInsertId().toInt();
                
                //Process playlist entries
                importPlaylist(xml, query_insert_to_playlist_tracks, playlist_id);
                
            }
        }
    }
    
    m_database.commit();
    m_childModel.setRootItem(rootItem);
   
    if (xml.hasError()) {
        // do error handling
        qDebug() << "Cannot process Rhythmbox music collection";
        qDebug() << "XML ERROR: " << xml.errorString();
        return false;
    }
    db.close();                                        
                                            
}
void RhythmboxFeature::importTrack(QXmlStreamReader &xml, QSqlQuery &query)
{
    QString title;
    QString artist;
    QString album;
    QString year;
    QString genre;
    QString location;

    int bpm = 0;
    int bitrate = 0;

    //duration of a track
    int playtime = 0;
    int rating = 0;
    QString comment;
    QString tracknumber;
    
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if(xml.name() == "title"){
                title = xml.readElementText();
                continue;
            }
            if(xml.name() == "artist"){
                artist = xml.readElementText();
                continue;
            }
            if(xml.name() == "genre"){
                genre = xml.readElementText();
                continue;
            }
            if(xml.name() == "album"){
                album = xml.readElementText();
                continue;
            }
            if(xml.name() == "track-number"){
                tracknumber = xml.readElementText();
                continue;
            }
            if(xml.name() == "duration"){
                playtime = xml.readElementText().toInt();;
                continue;
            }
            if(xml.name() == "bitrate"){
                bitrate = xml.readElementText().toInt();
                continue;
            }
            if(xml.name() == "beats-per-minute"){
                bpm = xml.readElementText().toInt();
                continue;
            }
            if(xml.name() == "comment"){
                comment = xml.readElementText();
                continue;
            }
            if(xml.name() == "location"){
                location = xml.readElementText();
                location.remove("file://");
                QByteArray strlocbytes = location.toUtf8();
                QUrl locationUrl = QUrl::fromEncoded(strlocbytes);
                location = locationUrl.toLocalFile();
                continue;
            }
        }
        //exit the loop if we reach the closing <entry> tag
        if (xml.isEndElement() && xml.name() == "entry") {
            break;
        }
        
    }
    query.bindValue(":artist", artist);
    query.bindValue(":title", title);
    query.bindValue(":album", album);
    query.bindValue(":genre", genre);
    query.bindValue(":year", year);
    query.bindValue(":duration", playtime);
    query.bindValue(":location", location);
    query.bindValue(":rating", rating);
    query.bindValue(":comment", comment);
    query.bindValue(":tracknumber", tracknumber);
    query.bindValue(":bpm", bpm);
    query.bindValue(":bitrate", bitrate);

    bool success = query.exec();

    if (!success) {
        qDebug() << "SQL Error in rhythmboxfeature.cpp: line" << __LINE__ << " " << query.lastError();
        return;
    }

}
/** reads all playlist entries and executes a SQL statement **/
void RhythmboxFeature::importPlaylist(QXmlStreamReader &xml, QSqlQuery &query_insert_to_playlist_tracks, int playlist_id)
{
    while(!xml.atEnd())
    {
        //read next XML element
        xml.readNext(); 
        if(xml.isStartElement() && xml.name() == "location")
        {
            QString location = xml.readElementText();
            location.remove("file://");
            QByteArray strlocbytes = location.toUtf8();
            QUrl locationUrl = QUrl::fromEncoded(strlocbytes);
            location = locationUrl.toLocalFile();
            
            //get the ID of the file in the rhythmbox_library table
            int track_id = -1;
            QSqlQuery finder_query(m_database);
            finder_query.prepare("select id from rhythmbox_library where location=:path"); 
            finder_query.bindValue(":path", location);
            bool success = finder_query.exec();
                    
                    
            if(success){
                while (finder_query.next()) {
                    track_id = finder_query.value(finder_query.record().indexOf("id")).toInt();
                }
             }   
             else
                qDebug() << "SQL Error in RhythmboxFeature.cpp: line" << __LINE__ << " " << finder_query.lastError();
            
            query_insert_to_playlist_tracks.bindValue(":playlist_id", playlist_id);
            query_insert_to_playlist_tracks.bindValue(":track_id", track_id);
            
            success = query_insert_to_playlist_tracks.exec();
            
            if(!success){
                qDebug() << "SQL Error in RhythmboxFeature.cpp: line" << __LINE__ << " "
                             << query_insert_to_playlist_tracks.lastError();
                qDebug() << "trackid" << track_id;
                qDebug() << "playlis ID " << playlist_id;
                qDebug() << "-----------------";

            }
        }
        //Exit the the loop if we reach the closing <playlist> tag
        if(xml.isEndElement() && xml.name() == "playlist")
        {
            break;
        }
    }

}
void RhythmboxFeature::clearTable(QString table_name)
{
    QSqlQuery query(m_database);
    query.prepare("delete from "+table_name);
    bool success = query.exec();
    
    if(!success)
        qDebug() << "Could not delete remove old entries from table " << table_name << " : " << query.lastError();
    else
        qDebug() << "Rhythmbox table entries of '" << table_name <<"' have been cleared.";
}
