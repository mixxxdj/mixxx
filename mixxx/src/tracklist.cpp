#include <qstring.h>
#include <qdom.h>
#include <qptrlist.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qdir.h>

#include "tracklist.h"
#include "trackinfoobject.h"

TrackList::TrackList( const QString _sDirectory )
{
	sDirectory = _sDirectory;

	// Initialize xml file:
	QFile opmlFile( sDirectory + "/tracklist.xml" );

	
    if ( !opmlFile.open( IO_ReadWrite ) ) {
        QMessageBox::critical( 0,
                tr( "Critical Error" ),
                tr( "Cannot open file %1" ).arg( sDirectory + "/tracklist.xml" ) );
        return;
    }

    if ( !domXML.setContent( &opmlFile ) ) {
        QMessageBox::critical( 0,
                tr( "Critical Error" ),
                tr( "Parsing error for file %1" ).arg( sDirectory + "/tracklist.xml" ) );
        opmlFile.close();
        return;
    }
    opmlFile.close();

    // Get all the tracks from the xml file:
    QDomElement root = domXML.documentElement();
    QDomNode node;
    node = root.firstChild();
    while ( !node.isNull() ) {
        if ( node.isElement() && node.nodeName() == "track" ) {
            QDomElement header = node.toElement();
            QString sFilename = node.toElement().attribute( "filename" );
			// Create a new track:
			TrackInfoObject *Track;
			Track = new TrackInfoObject( sFilename );
			Track->ReadFromXML( header );
			// Append it to the list of tracks:
			lTracks.append( Track );
        }
        node = node.nextSibling();
    }

	// Run through all the files and add the new ones to the xml file:
	AddFiles( sDirectory );
}

TrackList::~TrackList()
{
	// Delete all the tracks:
}

void TrackList::WriteXML();

void TrackList::AddFiles(const char *path)
{    
	// First run through all directories:
    QDir dir(path);
    if (!dir.exists())
        qWarning( "Cannot find the directory %s.",path);
    else
    {
		dir.setFilter(QDir::Dirs);
        const QFileInfoList dir_list = *dir.entryInfoList();
        QFileInfoListIterator dir_it(dir_list);
        QFileInfo *d;
        dir_it += 2; // Traverse past "." and ".."
        while ((d=dir_it.current()))
        {
            AddFiles(d->filePath());
            ++dir_it;
        }

        // ... and then all the files:
        dir.setFilter(QDir::Files);
        dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3");
        const QFileInfoList *list = dir.entryInfoList();
        QFileInfoListIterator it(*list);        // create list iterator
        QFileInfo *fi;                          // pointer for traversing

        while ((fi=it.current()))
        {
			// Check if the file exists in the list:
			if (!FileExistsInList( fi->fileName() ))
			{
				// Insert a new file:
				TrackInfoObject *Track;
				Track = new TrackInfoObject( fi->fileName() );
				Track->Parse();
				lTracks.append( Track );
			}
            ++it;   // goto next list element
        }
	}
}

TrackInfoObject *TrackList::FileExistsInList( const QString sFilename )
{	
	TrackInfoObject *Track;
	Track = lTracks.first();
	while ((Track) && (Track->sFilename != sFilename) )
		Track = lTracks.next();

	return Track;
}

