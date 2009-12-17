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


LegacyLibraryImporter::LegacyLibraryImporter(TrackDAO& trackDao)
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
                //blah, can't figure out how to use an interator with QDomNodeList
                track = trackList.at(i);
                TrackInfoObject trackInfo(track);
                //Only add the track to the DB if the file exists on disk,
                //because Mixxx <= 1.7 had no logic to deal with detecting deleted
                //files.
                QFileInfo info(trackInfo.getLocation());
                if(info.exists())
                    trackDao.addTrack(&trackInfo);	    
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
