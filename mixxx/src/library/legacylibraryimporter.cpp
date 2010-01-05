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


LegacyLibraryImporter::LegacyLibraryImporter(TrackDAO& trackDao) : QObject(),
    m_trackDao(trackDao)
{
}

/** Upgrade from <= 1.7 library to 1.8 DB format */
void LegacyLibraryImporter::import()
{
    QString trackXML = QDir::homePath().append("/").append(SETTINGS_PATH).append("mixxxtrack.xml");
    QFile file(trackXML);

    QDomDocument doc("TrackList");

    if(!file.open(QIODevice::ReadOnly)) {
        //qDebug() << "Could not import legacy 1.7 XML library: " << trackXML;
        return;
    } else {

        QString* errorMsg = NULL;
        int* errorLine = NULL; 
        int* errorColumn = NULL;
    
        qDebug() << "Starting upgrade from 1.7 library...";

        if (doc.setContent(&file, false, errorMsg, errorLine, errorColumn)) {
            QDomNodeList trackList = doc.elementsByTagName("Track");
            QDomNode track;

            for (int i = 0; i < trackList.size(); i++) {
                //blah, can't figure out how to use an iterator with QDomNodeList
                track = trackList.at(i);
                TrackInfoObject trackInfo17(track);
                //Only add the track to the DB if the file exists on disk,
                //because Mixxx <= 1.7 had no logic to deal with detecting deleted
                //files.
                QFileInfo info(trackInfo17.getLocation());
                if(info.exists()) {
                    //Create a TrackInfoObject by directly parsing
                    //the actual MP3/OGG/whatever because 1.7 didn't parse
                    //genre and album tags (so the imported TIO doesn't have
                    //those fields).
                    emit(progress("Upgrading Mixxx 1.7 Library: " + trackInfo17.getTitle()));
                    TrackInfoObject trackInfoNew(trackInfo17.getLocation());
                    trackInfo17.setGenre(trackInfoNew.getGenre());
                    trackInfo17.setAlbum(trackInfoNew.getAlbum());
                    trackInfo17.setYear(trackInfoNew.getYear());
                    trackInfo17.setTrackNumber(trackInfoNew.getTrackNumber());
                    m_trackDao.addTrack(&trackInfo17);	    
                }
            }

            //now change the file to mixxxtrack.bak so that its not readded next time program loads
            file.copy(QDir::homePath().append("/").append(SETTINGS_PATH).append("mixxxtrack.bak"));
            file.remove();
        } else {
            qDebug() << errorMsg << " line: " << errorLine << " column: " << errorColumn;
        }
    }

    file.close();
}


LegacyLibraryImporter::~LegacyLibraryImporter()
{
}
