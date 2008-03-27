//
// C++ Implementation: trackimporter
//
// Description: Manager class for the different playlist format parsers
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "trackcollection.h"
#include "trackplaylist.h"
#include "trackinfoobject.h"
#include "trackimporter.h"
//Parsers are going below here
#include "parserpls.h"
#include "parserm3u.h"
//Added by qt3to4:
#include <Q3PtrList>
#include <QtGui>
#include <QtCore>

TrackImporter::TrackImporter(QWidget * parent,TrackCollection * pCollection)
{

    m_pwParent = parent;
    m_pCollection = pCollection;
    m_psLocations = new Q3PtrList<QString>;
    this->setup();
}



TrackImporter::~TrackImporter()
{
    clearTracks();
    delete m_psLocations;
    delete m_pPlsparser;
}
/**Sets up the Parsers**/
void TrackImporter::setup()
{

    //Init the Parsers
    m_pPlsparser = new ParserPls();
    m_pM3uparser = new ParserM3u();

}
/** Clears m_psLocations**/
void TrackImporter::clearTracks()
{

    while(!m_psLocations->isEmpty())
        m_psLocations->removeFirst();

}

/**Displays a file chooser dialog for the user**/
QString TrackImporter::showChooser()
{
    QString sFilename = QFileDialog::getOpenFileName(
        m_pwParent, "Import playlist", "",
        "Winamp Playlists (*.pls *.m3u)");

    return sFilename;
}

void TrackImporter::parseTracks(QString sName)
{
    if(sName.endsWith(".pls")){

        m_psLocations = m_pPlsparser->parse(sName);

    } else if(sName.endsWith(".m3u")){

        m_psLocations = m_pM3uparser->parse(sName);

    } else{
        qDebug() << "Importer: File is not a playlist!";
    }

}

/** Auxiliary function**/
QString TrackImporter::splitFilepath(QString sFilepath)
{

    QStringList lst(sFilepath.split("/"));
    QStringList::Iterator lstIt = lst.end();

    return((*--lstIt));

}

/**Being called to set the tracks from the parser to the StringList**/
void TrackImporter::setTracks(Q3PtrList<QString> * pLocations)
{
    m_psLocations = pLocations;
}

/**Being called to create a new Playlist with name sName**/
TrackPlaylist * TrackImporter::importPlaylist(QString sName)
{
    TrackPlaylist * pPlaylist = new TrackPlaylist(m_pCollection, sName);
    TrackInfoObject * pTrack;

    QString sFilename = showChooser();
    if (sFilename.isEmpty())
        return 0;

    if(!m_psLocations ==0)
        clearTracks();
    parseTracks(sFilename);

    if(m_psLocations == 0 || m_psLocations->isEmpty()){
        QMessageBox::information( m_pwParent, "Track import error",
                                 "File did not contain any valid local filepaths.\n"
                                 "(Format not recognized or streaming content)");
        return 0; //Return zero if there were no locations parsed.
    }
    while(!m_psLocations->isEmpty())
    {
        //qDebug() << "Importer: Working on: " << (*m_psLocations->first());
        pTrack = m_pCollection->getTrack((*m_psLocations->first()));

        if(pTrack){

            pPlaylist->addTrack(pTrack);
            m_psLocations->removeFirst();

        } else{
            qDebug() << "Importer: Track was NULL, maybe its file is malformed (" << (*m_psLocations->first()) << ")";
            m_psLocations->removeFirst();
        }


    }
    return pPlaylist;

}
