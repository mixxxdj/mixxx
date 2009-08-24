#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>

#include "trackcollection.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "defs_audiofiles.h"

TrackCollection::TrackCollection()
{
    bCancelLibraryScan = 0;
    
 	//Create the SQLite database connection.
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    
    qDebug() << QSqlDatabase::drivers();
    
    m_db.setHostName("localhost");
    m_db.setDatabaseName("mixxxdb");
    m_db.setUserName("mixxx");
    m_db.setPassword("mixxx");
    bool ok = m_db.open();
    qDebug() << __FILE__ << "DB status:" << ok;
    qDebug() << m_db.lastError();
    
    //Check for tables and create them if missing
    checkForTables();
}

TrackCollection::~TrackCollection()
{
    m_db.close();    
}

bool TrackCollection::checkForTables()
{
    if (!m_db.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
                              qApp->tr("Unable to establish a database connection.\n"
                                       "Mixxx requires QT with SQLite support. Please read "
                                       "the Qt SQL driver documentation for information how "
                                       "to build it.\n\n"
                                       "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }
 
    //TODO: Check if the table exists...
    //      If it doesn't exist, create it.
 
 	QSqlQuery query;
 	//Little bobby tables
    query.exec("CREATE TABLE library (id INTEGER primary key, "
               "artist varchar(48), title varchar(48), "
               "album varchar(48), year varchar(16), "
               "genre varchar(32), tracknumber varchar(3), "
 			   "filename varchar(512), location varchar(512), "
 			   "comment varchar(20), url varchar(256), "
 			   "duration integer, length_in_bytes integer, "
     		   "bitrate integer, samplerate integer, "
     		   "cuepoint integer, bpm float, "
     		   "wavesummaryhex blob, "
 			   "channels integer)");
 			   
    query.exec("CREATE TABLE Playlists (id INTEGER primary key, "
           "name varchar(48), date_created datetime, "
           "date_modified datetime)");	

    query.exec("CREATE TABLE PlaylistTracks (id INTEGER primary key, "
           "playlist_id INTEGER REFERENCES Playlists(id),"
           "track_id INTEGER REFERENCES library(id), "
           "position INTEGER)");	


    //Create an example playlist
    query.prepare("INSERT INTO Playlists (name)"
                  "VALUES (:name)");
 				 // ":date_created, :date_modified)");
    query.bindValue(":name", "Example Playlist #1");
    query.exec();
 	
    query.prepare("INSERT INTO PlaylistTracks (playlist_id, track_id, position)"
                  "VALUES (:playlist_id, :track_id, :position)");
    query.bindValue(":playlist_id", 1);
    query.bindValue(":track_id", 1);
    query.bindValue(":position", 0);        
    query.exec();           	   
           	   
    return true;
}
 
void TrackCollection::addTrack(QString location)
{
	QFileInfo file(location);
	TrackInfoObject * pTrack = new TrackInfoObject(file.absoluteFilePath());
 	if (pTrack) {
 		//Add the song to the database.
 		this->addTrack(pTrack);
 		delete pTrack;
 	}
  }
  
  void TrackCollection::addTrack(TrackInfoObject * pTrack)
  {
 
 	//qDebug() << "TrackCollection::addTrack(), inserting into DB";
 
    //Start the transaction
    QSqlDatabase::database().transaction();
 
 	QSqlQuery query;
    query.prepare("INSERT INTO library (artist, title, album, year, genre, tracknumber, filename, "
 				  "location, comment, url, duration, length_in_bytes, "
 				  "bitrate, samplerate, cuepoint, bpm, wavesummaryhex, "
 				  "channels) "
                  "VALUES (:artist, "
 				  ":title, :album, :year, :genre, :tracknumber, :filename, "
 				  ":location, :comment, :url, :duration, :length_in_bytes, "
 				  ":bitrate, :samplerate, :cuepoint, :bpm, :wavesummaryhex, "
                  ":channels)");
    //query.bindValue(":id", 1001);
    query.bindValue(":artist", pTrack->getArtist());
    query.bindValue(":title", pTrack->getTitle());
    query.bindValue(":album", pTrack->getAlbum());
    query.bindValue(":year", pTrack->getYear());
    query.bindValue(":genre", pTrack->getGenre());
    query.bindValue(":tracknumber", pTrack->getTrackNumber());
    query.bindValue(":filename", pTrack->getFilename());
    query.bindValue(":location", pTrack->getLocation());
    query.bindValue(":comment", pTrack->getComment());
    query.bindValue(":url", pTrack->getURL());
    query.bindValue(":duration", pTrack->getDuration());
    query.bindValue(":length_in_bytes", pTrack->getLength());
    query.bindValue(":bitrate", pTrack->getBitrate());
    query.bindValue(":samplerate", pTrack->getSampleRate());
    query.bindValue(":cuepoint", pTrack->getCuePoint());
    query.bindValue(":bpm", pTrack->getBpm());
    query.bindValue(":wavesummaryhex", *(pTrack->getWaveSummary()));
    //query.bindValue(":timesplayed", pTrack->getCuePoint());
    query.bindValue(":channels", pTrack->getChannels());
 
    query.exec();
 
    //Commit the transaction
    QSqlDatabase::database().commit();
 
    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }
 
}
 
/** Check if a track exists in the database already.
    @param file_location The full path to the track on disk, including the filename.
    @return true if the track is found in the database, false otherwise.
*/
bool TrackCollection::trackExistsInDatabase(QString file_location)
{
 	QSqlQuery query("SELECT * FROM library WHERE location==\"" + file_location + "\"");
 
    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }
 	int numRecords = 0;
    while (query.next()) {
 		numRecords++;
    }
 	if (numRecords > 0)
 		return true;
 	return false;
}
 
QSqlDatabase TrackCollection::getDatabase()
{
 	return m_db;
  }
  
  /** Removes a track from the library track collection. */
void TrackCollection::removeTrack(QString location)
{    
    QSqlQuery query("DELETE FROM library WHERE location==\"" + location + "\"");
    query.exec();
     
    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }
}
 
TrackInfoObject *TrackCollection::getTrackFromDB(QSqlQuery &query)
{
 	TrackInfoObject* track = NULL;
 	if (!query.isValid()) {
 		//query.exec();
 	}
     
    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }
     
    while (query.next()) {
     	track = new TrackInfoObject();
        QString artist = query.value(query.record().indexOf("artist")).toString();
        QString title = query.value(query.record().indexOf("title")).toString();
        QString album = query.value(query.record().indexOf("album")).toString();
        QString year = query.value(query.record().indexOf("year")).toString();
        QString genre = query.value(query.record().indexOf("genre")).toString();
        QString tracknumber = query.value(query.record().indexOf("tracknumber")).toString();
        QString filename = query.value(query.record().indexOf("filename")).toString();
        QString location = query.value(query.record().indexOf("location")).toString();
        QString comment = query.value(query.record().indexOf("comment")).toString();
        QString url = query.value(query.record().indexOf("url")).toString();
        int duration = query.value(query.record().indexOf("duration")).toInt();
        int length = query.value(query.record().indexOf("length_in_bytes")).toInt();
        int bitrate = query.value(query.record().indexOf("bitrate")).toInt();
        int samplerate = query.value(query.record().indexOf("samplerate")).toInt();
        int cuepoint = query.value(query.record().indexOf("cuepoint")).toInt();
        QString bpm = query.value(query.record().indexOf("bpm")).toString();
        QByteArray* wavesummaryhex = new QByteArray(query.value(query.record().indexOf("wavesummaryhex")).toByteArray());
        //int timesplayed = query.value(query.record().indexOf("timesplayed")).toInt();
        int channels = query.value(query.record().indexOf("channels")).toInt();
 
 
        track->setArtist(artist);
        track->setTitle(title);
        track->setAlbum(album);
        track->setYear(year);
        track->setGenre(genre);
        track->setTrackNumber(tracknumber);
        track->setFilename(filename);
        track->setLocation(location);
        track->setComment(comment);
        track->setURL(url);
        track->setDuration(duration);
        track->setLength(length);
        track->setBitrate(bitrate);
        track->setSampleRate(samplerate);
        track->setCuePoint((float)cuepoint);
        track->setBpm(bpm.toFloat());
        track->setWaveSummary(wavesummaryhex, NULL, false);
        //track->setTimesPlayed //Doesn't exist wtfbbq
        track->setChannels(channels);
         
    }
    return track;
 
 
}
 
/** This function is for debugging only!!!! */
QList<TrackInfoObject*> TrackCollection::dumpDB()
{
 	QList<TrackInfoObject*> tracks;
 	TrackInfoObject* track = NULL;
 	
 	/*
      qDebug() << "Dumping TrackCollection database...";
 	
      //FIXME: This only dumps the last record... why?
      QSqlQuery query("SELECT * FROM library");
      do {
      track = getTrackFromDB(query);
      tracks.push_back(track);
      if (track) {
      qDebug() << track->getTitle();
      qDebug() << track->getFilename();
      }
      } while (track != NULL);
    */
     
    return tracks;
}
 
void TrackCollection::scanPath(QString path)
{
 	//qDebug() << "TrackCollection::scanPath(" << path << ")";
 	bCancelLibraryScan = false; //Reset the flag
 
    emit(startedLoading());
 	QFileInfoList files;
 
 	//Check to make sure the path exists.
 	QDir dir(path);
 	if (dir.exists()) {
 		files = dir.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot); //TODO: entryInfoList takes a sorting parameter. Might want to use that.
 	} else {
 		qDebug() << "Error: Scan path does not exist." << path;
 		return;
 	}
 
 	//The directory exists, so get a list of the contents of the directory and go through it.
 	QListIterator<QFileInfo> it(files);
 	while (it.hasNext())
        {
    	    QFileInfo file = it.next(); //TODO: THIS IS SLOW!
            //If a flag was raised telling us to cancel the library scan then stop.
            if (bCancelLibraryScan == 1)
                {
                    return;
                }
 
            if (file.isDir()) { //Recurse into directories
                emit(progressLoading(file.fileName()));
                scanPath(file.absoluteFilePath());
            }
            else if (file.fileName().count(QRegExp(MIXXX_SUPPORTED_AUDIO_FILETYPES_REGEX, Qt::CaseInsensitive))) {
                //If the file already exists in the database, continue and go on to the next file.
                if (this->trackExistsInDatabase(file.absoluteFilePath()))
                    {
                        continue;
                    }
                //Load the song into a TrackInfoObject.
                emit(progressLoading(file.fileName()));
                //qDebug() << "Loading" << file.fileName();
 			
                TrackInfoObject * pTrack = new TrackInfoObject(file.absoluteFilePath());
                if (pTrack) {
                    //Add the song to the database.
                    this->addTrack(pTrack);
                    delete pTrack;
                }
            } else {
                //qDebug() << "Skipping" << file.fileName() << "because it did not match thesupported audio files filter:" << MIXXX_SUPPORTED_AUDIO_FILETYPES_REGEX;
            }
 		
        }
    emit(finishedLoading());
 
}

  
  TrackInfoObject * TrackCollection::getTrack(QString location)
  {
    QSqlQuery query("SELECT * FROM library WHERE location==\"" + location + "\"");
 	TrackInfoObject* track = getTrackFromDB(query);
 
    return track;
}
 
/** Saves a track's info back to the database */
void TrackCollection::updateTrackInDatabase(TrackInfoObject* pTrack)
{
 	qDebug() << "Updating track" << pTrack->getInfo() << "in database...";
 	
 	QSqlQuery query;
 	//Update everything but "location", since that's what we identify the track by.
    query.prepare("UPDATE library "
                  "SET artist=:artist, "
 				  "title=:title, album=:album, :year=filename=:filename, "
 				  "comment=:comment, url=:url, duration=:duration, "
 				  "length_in_bytes=:length_in_bytes, "
 				  "bitrate=:bitrate, samplerate=:samplerate, cuepoint=:cuepoint, "
 				  "bpm=:bpm, wavesummaryhex=:wavesummaryhex, "
                  "channels=:channels "
                  "WHERE location==\"" + pTrack->getLocation() + "\"");
    //query.bindValue(":id", 1001);
    query.bindValue(":artist", pTrack->getArtist());
    query.bindValue(":title", pTrack->getTitle());
    query.bindValue(":album", pTrack->getAlbum());
    query.bindValue(":year", pTrack->getYear());
    query.bindValue(":genre", pTrack->getGenre());
    query.bindValue(":tracknumber", pTrack->getTrackNumber());
    query.bindValue(":filename", pTrack->getFilename());
    query.bindValue(":comment", pTrack->getComment());
    query.bindValue(":url", pTrack->getURL());
    query.bindValue(":duration", pTrack->getDuration());
    query.bindValue(":length_in_bytes", pTrack->getLength());
    query.bindValue(":bitrate", pTrack->getBitrate());
    query.bindValue(":samplerate", pTrack->getSampleRate());
    query.bindValue(":cuepoint", pTrack->getCuePoint());
    query.bindValue(":bpm", pTrack->getBpm());
    query.bindValue(":wavesummaryhex", *(pTrack->getWaveSummary()));
    //query.bindValue(":timesplayed", pTrack->getCuePoint());
    query.bindValue(":channels", pTrack->getChannels());
    //query.bindValue(":location", pTrack->getLocation());
 
    query.exec();
     
    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }
}
 
void TrackCollection::slotCancelLibraryScan()
{
 	//Note that this does not need to be protected by a mutex since integer operations are atomic.
 	bCancelLibraryScan = 1;
  }
