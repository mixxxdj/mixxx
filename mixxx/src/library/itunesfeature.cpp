#include <QMessageBox>
#include <QtDebug>
#include <QXmlStreamReader>
#include <QDesktopServices>
#include "library/itunesfeature.h"

#include "library/itunestrackmodel.h"
#include "library/itunesplaylistmodel.h"


ITunesFeature::ITunesFeature(QObject* parent, TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pTrackCollection(pTrackCollection),
          m_database(pTrackCollection->getDatabase()) {
    m_pITunesTrackModel = new ITunesTrackModel(this, m_pTrackCollection);
    m_pITunesPlaylistModel = new ITunesPlaylistModel(this, m_pTrackCollection);
    m_isActivated = false;
}

ITunesFeature::~ITunesFeature() {
    delete m_pITunesTrackModel;
    delete m_pITunesPlaylistModel;
}

bool ITunesFeature::isSupported() {
    return QFile::exists(getiTunesMusicPath());
}


QVariant ITunesFeature::title() {
    return tr("iTunes");
}

QIcon ITunesFeature::getIcon() {
    return QIcon(":/images/library/ic_library_itunes.png");
}

void ITunesFeature::activate() {
    //qDebug("ITunesFeature::activate()");

    if (!m_isActivated) {
        if (QMessageBox::question(
            NULL,
            tr("Load iTunes Library?"),
            tr("Would you like to load your iTunes library?"),
            QMessageBox::Ok,
            QMessageBox::Cancel)
            == QMessageBox::Cancel) {
            return;
        }

        if (importLibrary(getiTunesMusicPath())) {
            m_isActivated =  true;
        } else {
            QMessageBox::warning(
                NULL,
                tr("Error Loading iTunes Library"),
                tr("There was an error loading your iTunes library. Some of your iTunes tracks or
playlists may not have loaded."));
        }

        //Sort the playlists since in iTunes they are sorted, too.
        //list.sort();

        m_childModel.setStringList(m_playlists);
    }

    emit(showTrackModel(m_pITunesTrackModel));
}

void ITunesFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "ITunesFeature::activateChild()" << index;
    QString playlist = index.data().toString();
    qDebug() << "Activating " << playlist;
    m_pITunesPlaylistModel->setPlaylist(playlist);
    emit(showTrackModel(m_pITunesPlaylistModel));
}

QAbstractItemModel* ITunesFeature::getChildModel() {
    return &m_childModel;
}

void ITunesFeature::onRightClick(const QPoint& globalPos) {
}

void ITunesFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
}

bool ITunesFeature::dropAccept(QUrl url) {
    return false;
}

bool ITunesFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool ITunesFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool ITunesFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

QString ITunesFeature::getiTunesMusicPath() {
    QString musicFolder;
#if defined(__APPLE__)
    musicFolder = QDesktopServices::storageLocation(QDesktopServices::MusicLocation) + "/iTunes/iTunes Music Library.xml";
#elif defined(__WINDOWS__)
    musicFolder = QDesktopServices::storageLocation(QDesktopServices::MusicLocation) + "\\iTunes\\iTunes Music Library.xml";
#elif defined(__LINUX__)
		musicFolder = QDir::homePath() + "/iTunes Music Library.xml";
#else
		musicFolder = "";
#endif
    qDebug() << "ITunesLibrary=[" << musicFolder << "]";
    return musicFolder;
}
bool ITunesFeature::importLibrary(QString file) {
    //Delete all table entries of Traktor feature
    m_database.transaction();
    clearTable("itunes_playlist_tracks");
    clearTable("itunes_library");
    clearTable("itunes_playlists");
    m_database.commit();

    qDebug() << "Import iTunes library";

    m_database.transaction();

    //Parse iTunes XML file using SAX (for performance)
    QFile itunes_file(file);
    if (!itunes_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open iTunes music collection";
        return false;
    }
    QXmlStreamReader xml(&itunes_file);

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "key") {
                if (xml.readElementText() == "Tracks") {
                    parseTracks(xml);
                    qDebug() << "Finished Parsing TrackList";
                } else if (xml.readElementText() == "Playlists") {
                    //parse all the playlists here and exit the loop afterwards
                    parsePlaylists(xml);
                    qDebug() << "Finished Parsing Playlists";
                }
            }
        }
    }

    itunes_file.close();

    // Even if an error occured, commit the transaction. The file may have been
    // half-parsed.
    m_database.commit();
    m_pITunesTrackModel->select();

    if (xml.hasError()) {
        // do error handling
        qDebug() << "Cannot process iTunes music collection";
        qDebug() << "XML ERROR: " << xml.errorString();
        return false;
    }
    return true;
}

void ITunesFeature::parseTracks(QXmlStreamReader &xml) {
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO itunes_library (id, artist, title, album, year, genre, comment, tracknumber,"
                  "bpm, bitrate,"
                  "duration, location,"
                  "rating ) "
                  "VALUES (:id, :artist, :title, :album, :year, :genre, :comment, :tracknumber,"
                  ":bpm, :bitrate,"
                  ":duration, :location," ":rating )");


    bool in_container_dictionary = false;
    bool in_track_dictionary = false;

    qDebug() << "Parse Tracks";

    //read all sunsequent <dict> until we reach the closing ENTRY tag
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == "dict") {
                if (!in_track_dictionary && !in_container_dictionary) {
                    in_container_dictionary = true;
                    continue;
                } else if (in_container_dictionary && !in_track_dictionary) {
                    //We are in a <dict> tag that holds track information
                    in_track_dictionary = true;
                    //Parse track here
                    parseTrack(xml, query);
                }
            }
        }

        if (xml.isEndElement() && xml.name() == "dict") {
            if (in_track_dictionary && in_container_dictionary) {
                in_track_dictionary = false;
                continue;
            } else if (in_container_dictionary && !in_track_dictionary) {
                // Done parsing tracks.
                in_container_dictionary = false;
                break;
            }
        }
    }
}

void ITunesFeature::parseTrack(QXmlStreamReader &xml, QSqlQuery &query) {
    //qDebug() << "----------------TRACK-----------------";
    int id = -1;
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
            if (xml.name() == "key") {
                QString key = xml.readElementText();
                QString content =  "";

                if (xml.readNextStartElement()) {
                    content = xml.readElementText();
                }

                //qDebug() << "Key: " << key << " Content: " << content;

                if (key == "Track ID") {
                    id = content.toInt();
                    continue;
                }
                if (key == "Name") {
                    title = content;
                    continue;
                }
                if (key == "Artist") {
                    artist = content;
                    continue;
                }
                if (key == "Album") {
                    album = content;
                    continue;
                }
                if (key == "Genre") {
                    genre = content;
                    continue;
                }
                if (key == "BPM") {
                    bpm = content.toInt();
                    continue;
                }
                if (key == "Bit Rate") {
                    bitrate =  content.toInt();
                    continue;
                }
                if (key == "Comments") {
                    comment = content;
                    continue;
                }
                if (key == "Total Time") {
                    playtime = (content.toInt() / 1000);
                    continue;
                }
                if (key == "Year") {
                    year = content;
                    continue;
                }
                if (key == "Location") {
                    QByteArray strlocbytes = content.toUtf8();
                    location = QUrl::fromEncoded(strlocbytes).toLocalFile();
                    /*
                     * Strip the crappy file://localhost/ from the URL and
                     * format URL as in method ITunesTrackModel::parseTrackNode(QDomNode songNode)
                     */
#if defined(__WINDOWS__)
                    location.remove("//localhost/");
#else
                    location.remove("//localhost");
#endif
                    continue;
                }
                if (key == "Track Number") {
                    tracknumber = content;
                    continue;
                }
                if (key == "Rating") {
                    //value is an integer and ranges from 0 to 100
                    rating = (content.toInt() / 20);
                    continue;
                }
            }
        }
        //exit loop on closing </dict>
        if (xml.isEndElement() && xml.name() == "dict") {
            break;
        }
    }
    /* If we reach the end of <dict>
     * Save parsed track to database
     */
    query.bindValue(":id", id);
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
        qDebug() << "SQL Error in itunesfeature.cpp: line" << __LINE__ << " " << query.lastError();
        return;
    }
}

void ITunesFeature::parsePlaylists(QXmlStreamReader &xml) {
    qDebug() << "Parse Playlists";

    QSqlQuery query_insert_to_playlists(m_database);
    query_insert_to_playlists.prepare("INSERT INTO itunes_playlists (id, name) "
                                      "VALUES (:id, :name)");

    QSqlQuery query_insert_to_playlist_tracks(m_database);
    query_insert_to_playlist_tracks.prepare("INSERT INTO itunes_playlist_tracks (playlist_id, track_id) "
                                            "VALUES (:playlist_id, :track_id)");

    while (!xml.atEnd()) {
        xml.readNext();
        //We process and iterate the <dict> tags holding playlist summary information here
        if (xml.isStartElement() && xml.name() == "dict") {
            parsePlaylist(xml, query_insert_to_playlists, query_insert_to_playlist_tracks);
            continue;
        }
        if (xml.isEndElement()) {
            if (xml.name() == "array")
                break;
        }
    }
}

void ITunesFeature::parsePlaylist(QXmlStreamReader &xml, QSqlQuery &query_insert_to_playlists,
                                  QSqlQuery &query_insert_to_playlist_tracks) {
    //qDebug() << "Parse Playlist";

    QString playlistname;
    int playlist_id = -1;
    int track_reference = -1;
    //indicates that we haven't found the <
    bool isSystemPlaylist = false;

    QString key;


    //We process and iterate the <dict> tags holding playlist summary information here
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {

            if (xml.name() == "key") {
                QString key = xml.readElementText();
                /*
                 * The rules are processed in sequence
                 * That is, XML is ordered.
                 * For iTunes Playlist names are always followed by the ID.
                 * Afterwars the playlist entries occur
                 */
                if (key == "Name") {
                    xml.readNextStartElement();
                    playlistname = xml.readElementText();
                    continue;
                }
                //When parsing the ID, the playlistname has already been found
                if (key == "Playlist ID") {
                    xml.readNextStartElement();
                    playlist_id = xml.readElementText().toInt();
                    continue;
                }
                //Hide playlists that are system playlists
                if (key == "Master" || key == "Movies" || key == "TV Shows" || key == "Music" ||
                   key == "Books" || key == "Purchased") {
                    isSystemPlaylist = true;
                    continue;
                }

                if (key == "Playlist Items") {
                    //if the playlist is prebuild don't hit the database
                    if (isSystemPlaylist) continue;
                    query_insert_to_playlists.bindValue(":id", playlist_id);
                    query_insert_to_playlists.bindValue(":name", playlistname);

                    bool success = query_insert_to_playlists.exec();
                    if (!success) {
                        qDebug() << "SQL Error in ITunesTableModel.cpp: line" << __LINE__
                                 << " " << query_insert_to_playlists.lastError();
                        return;
                    }
                    //for the child model
                    m_playlists << playlistname;

                }
                /*
                 * When processing playlist entries, playlist name and id have already been processed and persisted
                 */
                if (key == "Track ID") {
                    track_reference = -1;

                    xml.readNextStartElement();
                    track_reference = xml.readElementText().toInt();

                    query_insert_to_playlist_tracks.bindValue(":playlist_id", playlist_id);
                    query_insert_to_playlist_tracks.bindValue(":track_id", track_reference);

                    //Insert tracks if we are not in a pre-build playlist
                    if (!isSystemPlaylist && !query_insert_to_playlist_tracks.exec()) {
                        qDebug() << "SQL Error in ITunesFeature.cpp: line" << __LINE__ << " "
                                 << query_insert_to_playlist_tracks.lastError();
                        qDebug() << "trackid" << track_reference;
                        qDebug() << "playlistname; " << playlistname;
                        qDebug() << "-----------------";
                    }
                }
            }
        }
        if (xml.isEndElement()) {
            if (xml.name() == "array") {
                //qDebug() << "exit playlist";
                break;
            }
        }
    }
}

void ITunesFeature::clearTable(QString table_name) {
    QSqlQuery query(m_database);
    query.prepare("delete from "+table_name);
    bool success = query.exec();

    if (!success)
        qDebug() << "Could not delete remove old entries from table " << table_name << " : " << query.lastError();
    else
        qDebug() << "iTunes table entries of '" << table_name <<"' have been cleared.";
}
