/**************************************************************************
 * 1.7.x library upgrade code
 *
 * Description:
 *
 * Parse the mixxxtrack.xml file in order to create list of of songs to be
 * added to the sqlite database 1.8+ uses
 *
 * ***********************************************************************/

#include <QDomDocument>
#include <QDomNodeList>
#include <QDomNode>
#include <QDir>
#include <QFile>
#include <QDebug>
#include "trackinfoobject.h" //needed for importing 1.7.x library
#include "xmlparse.h" //needed for importing 1.7.x library
#include "legacylibraryimporter.h"

struct LegacyPlaylist
{
    QString name;
    QList<int> indexes;
};

void doNothing(TrackInfoObject*) {

}

LegacyLibraryImporter::LegacyLibraryImporter(TrackDAO& trackDao,
                                             PlaylistDAO& playlistDao) : QObject(),
    m_trackDao(trackDao),
    m_playlistDao(playlistDao)
{
}

/** Upgrade from <= 1.7 library to 1.8 DB format */
void LegacyLibraryImporter::import()
{
    // TODO(XXX) SETTINGS_PATH may change in new Mixxx Versions. Here we need
    // the SETTINGS_PATH from Mixxx V <= 1.7
    QString settingPath17 = QDir::homePath().append("/").append(SETTINGS_PATH);
    QString trackXML = settingPath17.append("mixxxtrack.xml");
    QFile file(trackXML);

    QDomDocument doc("TrackList");

    if(!file.open(QIODevice::ReadOnly)) {
        //qDebug() << "Could not import legacy 1.7 XML library: " << trackXML;
        return;
    }

    QString* errorMsg = NULL;
    int* errorLine = NULL;
    int* errorColumn = NULL;

    qDebug() << "Starting upgrade from 1.7 library...";

    QHash<int, QString> playlistHashTable; //Maps track indices onto track locations
    QList<LegacyPlaylist> legacyPlaylists; // <= 1.7 playlists

    if (doc.setContent(&file, false, errorMsg, errorLine, errorColumn)) {

        QDomNodeList playlistList = doc.elementsByTagName("Playlist");
        QDomNode playlist;
        for (int i = 0; i < playlistList.size(); i++)
        {
            LegacyPlaylist legPlaylist;
            playlist = playlistList.at(i);

            QString name = playlist.firstChildElement("Name").text();

            legPlaylist.name = name;

            //Store the IDs in the hash table so we can map them to track locations later,
            //and also store them in-order in a temporary playlist struct.
            QDomElement listNode = playlist.firstChildElement("List").toElement();
            QDomNodeList trackIDs = listNode.elementsByTagName("Id");
            for (int j = 0; j < trackIDs.size(); j++)
            {
                int id = trackIDs.at(j).toElement().text().toInt();
                if (!playlistHashTable.contains(id))
                    playlistHashTable.insert(id, "");
                legPlaylist.indexes.push_back(id); //Save this track id.
            }
            //Save this playlist in our list.
            legacyPlaylists.push_back(legPlaylist);
        }

        QDomNodeList trackList = doc.elementsByTagName("Track");
        QDomNode track;

        for (int i = 0; i < trackList.size(); i++) {
            //blah, can't figure out how to use an iterator with QDomNodeList
            track = trackList.at(i);
            TrackInfoObject trackInfo17(track);
            //Only add the track to the DB if the file exists on disk,
            //because Mixxx <= 1.7 had no logic to deal with detecting deleted
            //files.

            if (trackInfo17.exists()) {
                //Create a TrackInfoObject by directly parsing
                //the actual MP3/OGG/whatever because 1.7 didn't parse
                //genre and album tags (so the imported TIO doesn't have
                //those fields).
                emit(progress("Upgrading Mixxx 1.7 Library: " + trackInfo17.getTitle()));

                // Read the metadata we couldn't support in <1.8 from file.
                QFileInfo fileInfo(trackInfo17.getLocation());
                //Ensure we have the absolute file path stored
                trackInfo17.setLocation(fileInfo.absoluteFilePath());
                TrackInfoObject trackInfoNew(trackInfo17.getLocation());
                trackInfo17.setGenre(trackInfoNew.getGenre());
                trackInfo17.setAlbum(trackInfoNew.getAlbum());
                trackInfo17.setYear(trackInfoNew.getYear());
                trackInfo17.setType(trackInfoNew.getType());
                trackInfo17.setTrackNumber(trackInfoNew.getTrackNumber());
                trackInfo17.setKey(trackInfoNew.getKey());
                trackInfo17.setHeaderParsed(true);

                // Import the track's saved cue point if it is non-zero.
                float fCuePoint = trackInfo17.getCuePoint();
                if (fCuePoint != 0.0f) {
                    Cue* pCue = trackInfo17.addCue();
                    pCue->setType(Cue::CUE);
                    pCue->setPosition(fCuePoint);
                }

                // Provide a no-op deleter b/c this Track is on the stack.
                TrackPointer pTrack(&trackInfo17, &doNothing);
                m_trackDao.saveTrack(pTrack);

                //Check if this track is used in a playlist anywhere. If it is, save the
                //track location. (The "id" of a track in 1.8 is a database index, so it's totally
                //different. Using the track location is the best way for us to identify the song.)
                int id = trackInfo17.getId();
                if (playlistHashTable.contains(id))
                    playlistHashTable[id] = trackInfo17.getLocation();
            }
        }


        //Create the imported playlists
        QListIterator<LegacyPlaylist> it(legacyPlaylists);
        LegacyPlaylist current;
        while (it.hasNext())
        {
            current = it.next();
            emit(progress("Upgrading Mixxx 1.7 Playlists: " + current.name));

            //Create the playlist with the imported name.
            //qDebug() << "Importing playlist:" << current.name;
            int playlistId = m_playlistDao.createPlaylist(current.name);

            //For each track ID in the XML...
            QList<int> trackIDs = current.indexes;
            for (int i = 0; i < trackIDs.size(); i++)
            {
                QString trackLocation;
                int id = trackIDs[i];
                //qDebug() << "track ID:" << id;

                //Try to resolve the (XML's) track ID to a track location.
                if (playlistHashTable.contains(id)) {
                    trackLocation = playlistHashTable[id];
                    //qDebug() << "Resolved to:" << trackLocation;
                }

                //Get the database's track ID (NOT the XML's track ID!)
                int dbTrackId = m_trackDao.getTrackId(trackLocation);

                if (dbTrackId >= 0) {
                    // Add it to the database's playlist.
                    // TODO(XXX): Care if the append succeeded.
                    m_playlistDao.appendTrackToPlaylist(dbTrackId, playlistId);
                }
            }
        }

        QString upgrade_filename = settingPath17.append("DBUPGRADED");
        //now create stub so that the library is not readded next time program loads
        QFile upgradefile(upgrade_filename);
        if (!upgradefile.open(QIODevice::WriteOnly | QIODevice::Text))
            qDebug() << "Couldn't open" << upgrade_filename << "for writing";
        else
        {
            file.write("",0);
            file.close();
        }
    } else {
        qDebug() << errorMsg << " line: " << errorLine << " column: " << errorColumn;
    }

    file.close();
}


LegacyLibraryImporter::~LegacyLibraryImporter()
{
}
