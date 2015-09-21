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
#include <QDir>
#include <QFile>
#include <QtDebug>

#include "legacylibraryimporter.h"
#include "soundsourceproxy.h"
#include "util/xml.h" // needed for importing 1.7.x library
#include "upgrade.h"

namespace {

struct LegacyPlaylist {
    QString name;
    QList<TrackId> indexes;
};

QFileInfo parseFileInfo(const QDomNode &nodeHeader) {
    QDir filePathDir(XmlParse::selectNodeQString(nodeHeader, "Filepath"));
    QString fileName(XmlParse::selectNodeQString(nodeHeader, "Filename"));
    return QFileInfo(filePathDir, fileName);
}

} // anonymous namespace

LegacyLibraryImporter::LegacyLibraryImporter(TrackDAO& trackDao,
                                             PlaylistDAO& playlistDao) : QObject(),
    m_trackDao(trackDao),
    m_playlistDao(playlistDao) {
}

/** Upgrade from <= 1.7 library to 1.8 DB format */
void LegacyLibraryImporter::import() {
    // Here we need the SETTINGS_PATH from Mixxx V <= 1.7
    QString settingPath17 = Upgrade::mixxx17HomePath();

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

    QHash<TrackId, QString> playlistHashTable; // Maps track indices onto track locations
    QList<LegacyPlaylist> legacyPlaylists; // <= 1.7 playlists

    if (doc.setContent(&file, false, errorMsg, errorLine, errorColumn)) {

        QDomNodeList playlistList = doc.elementsByTagName("Playlist");
        QDomNode playlist;
        for (int i = 0; i < playlistList.size(); i++) {
            LegacyPlaylist legPlaylist;
            playlist = playlistList.at(i);

            QString name = playlist.firstChildElement("Name").text();

            legPlaylist.name = name;

            // Store the IDs in the hash table so we can map them to track locations later,
            // and also store them in-order in a temporary playlist struct.
            QDomElement listNode = playlist.firstChildElement("List").toElement();
            QDomNodeList trackIDs = listNode.elementsByTagName("Id");
            for (int j = 0; j < trackIDs.size(); j++) {
                TrackId trackId(trackIDs.at(j).toElement().text().toInt());
                if (!playlistHashTable.contains(trackId))
                    playlistHashTable.insert(trackId, "");
                legPlaylist.indexes.push_back(trackId); // Save this track id.
            }
            // Save this playlist in our list.
            legacyPlaylists.push_back(legPlaylist);
        }

        QDomNodeList trackNodeList = doc.elementsByTagName("Track");
        QDomNode trackNode;

        for (int i = 0; i < trackNodeList.size(); i++) {
            // blah, can't figure out how to use an iterator with QDomNodeList
            trackNode = trackNodeList.at(i);
            QFileInfo fileInfo(parseFileInfo(trackNode));

            // Only add the track to the DB if the file exists on disk,
            // because Mixxx <= 1.7 had no logic to deal with detecting deleted
            // files.
            if (fileInfo.exists()) {
                TrackPointer pTrack(TrackInfoObject::newTemporaryForFile(fileInfo));

                // Parse the actual MP3/OGG/whatever because 1.7 didn't parse
                // genre and album tags (so the imported TIO doesn't have
                // those fields).
                SoundSourceProxy(pTrack).loadTrackMetadata();

                // Import values from the Mixxx 1.7 library and overwrite the
                // values that have just been parsed from the file.
                importTrack(pTrack.data(), trackNode);

                emit(progress("Upgrading Mixxx 1.7 Library: " + pTrack->getTitle()));

                // Import the track's saved cue point if it is non-zero.
                float fCuePoint = pTrack->getCuePoint();
                if (fCuePoint != 0.0f) {
                    Cue* pCue = pTrack->addCue();
                    pCue->setType(Cue::CUE);
                    pCue->setPosition(fCuePoint);
                }

                m_trackDao.saveTrack(pTrack);

                // Check if this track is used in a playlist anywhere. If it is, save the
                // track location. (The "id" of a track in 1.8 is a database index, so it's totally
                // different. Using the track location is the best way for us to identify the song.)
                TrackId trackId(pTrack->getId());
                if (playlistHashTable.contains(trackId)) {
                    playlistHashTable[trackId] = pTrack->getLocation();
                }
            }
        }


        // Create the imported playlists
        QListIterator<LegacyPlaylist> it(legacyPlaylists);
        LegacyPlaylist current;
        while (it.hasNext()) {
            current = it.next();
            emit(progress("Upgrading Mixxx 1.7 Playlists: " + current.name));

            // Create the playlist with the imported name.
            //qDebug() << "Importing playlist:" << current.name;
            int playlistId = m_playlistDao.createPlaylist(current.name);

            // For each track ID in the XML...
            QList<TrackId> trackIds = current.indexes;
            for (int i = 0; i < trackIds.size(); i++)
            {
                QString trackLocation;
                TrackId trackId(trackIds[i]);
                //qDebug() << "track ID:" << id;

                // Try to resolve the (XML's) track ID to a track location.
                if (playlistHashTable.contains(trackId)) {
                    trackLocation = playlistHashTable[trackId];
                    //qDebug() << "Resolved to:" << trackLocation;
                }

                // Get the database's track ID (NOT the XML's track ID!)
                TrackId dbTrackId(m_trackDao.getTrackId(trackLocation));

                if (dbTrackId.isValid()) {
                    // Add it to the database's playlist.
                    // TODO(XXX): Care if the append succeeded.
                    m_playlistDao.appendTrackToPlaylist(dbTrackId, playlistId);
                }
            }
        }

        QString upgrade_filename = settingPath17.append("DBUPGRADED");
        // now create stub so that the library is not readded next time program loads
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

void LegacyLibraryImporter::importTrack(TrackInfoObject* pTrack, const QDomNode &nodeHeader) {
    // It is safe to directly access the members of pTrack,
    // because it has just been created and there are no
    // references to it yet!

    // TODO(uklotzde): The following setters will soon be replaced
    // by directly accessing the TrackMetadata member of the track.
    pTrack->setTitle(XmlParse::selectNodeQString(nodeHeader, "Title"));
    pTrack->setArtist(XmlParse::selectNodeQString(nodeHeader, "Artist"));
    pTrack->setComment(XmlParse::selectNodeQString(nodeHeader, "Comment"));
    pTrack->setDuration(XmlParse::selectNodeQString(nodeHeader, "Duration").toInt());
    pTrack->setSampleRate(XmlParse::selectNodeQString(nodeHeader, "SampleRate").toInt());
    pTrack->setChannels(XmlParse::selectNodeQString(nodeHeader, "Channels").toInt());
    pTrack->setBitrate(XmlParse::selectNodeQString(nodeHeader, "Bitrate").toInt());
    pTrack->setReplayGain(XmlParse::selectNodeQString(nodeHeader, "replaygain").toDouble());

    // Mixxx <1.8 recorded track IDs in mixxxtrack.xml, but we are going to
    // ignore those. Tracks will get a new ID from the database.
    //m_iId = XmlParse::selectNodeQString(nodeHeader, "Id").toInt();

    pTrack->m_sType = XmlParse::selectNodeQString(nodeHeader, "Type");
    pTrack->m_iTimesPlayed = XmlParse::selectNodeQString(nodeHeader, "TimesPlayed").toInt();
    pTrack->m_fCuePoint = XmlParse::selectNodeQString(nodeHeader, "CuePoint").toFloat();
}
