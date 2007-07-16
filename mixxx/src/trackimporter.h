//
// C++ Interface: trackimporter
//
// Description:
//
//
// Author: Ingo Kossyk <kossyki@cs.tu-berlin.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef TRACKIMPORTER_H
#define TRACKIMPORTER_H

#include <qobject.h>
#include <qstring.h>
#include <q3ptrlist.h>
#include <q3filedialog.h>
#include <qmessagebox.h>
#include "parserpls.h"
#include "parserm3u.h"
#include "trackcollection.h"
#include "trackplaylist.h"

class ParserPls;
class ParserM3u;
/**
@author Ingo Kossyk (kossyki@cs.tu-berlin.de)
**/

class TrackImporter : public QObject
{
    Q_OBJECT

public:
    TrackImporter(QWidget * parent,TrackCollection * pCollection);
    ~TrackImporter();

    /**Can be called to import a new Playlist with name sName**/
    TrackPlaylist * importPlaylist(QString);

private:
    /** Sets up the parsers **/
    void setup();
    /**Being called to parse the tracks corresponding to the file's extension**/
    void parseTracks(QString);
    /**Being called to set the tracks from the parser to the StringList**/
    void setTracks(Q3PtrList<QString> *);

    /**Displays a file chooser dialog for the user**/
    QString showChooser();
    /** Clears m_psLocations**/
    void clearTracks();

    /**Splits filepaths at "/" and returns filename only **/
    QString splitFilepath(QString);

    /**Pointer to a list of the tracklocations**/
    Q3PtrList<QString> *m_psLocations;
    /**Pointer to the Main Collection class**/
    TrackCollection * m_pCollection;

    /**Pointer to the Parent widget**/
    QWidget * m_pwParent;

    /**Pointers to the different Parsers go below here**/
    ParserPls * m_pPlsparser;	//Pointer to the PLSPARSER (*.pls)
    ParserM3u * m_pM3uparser;   //Pointer to the M3UPARSER (*.m3u)

};

#endif
