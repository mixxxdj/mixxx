#include <qstring.h>
#include <qdom.h>
#include <qptrlist.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qtextstream.h>

#include "tracklist.h"
#include "trackinfoobject.h"

TrackList::TrackList( const QString sDirectory )
{
	m_sDirectory = sDirectory;

	// Initialize xml file:
	QFile opmlFile( m_sDirectory + "/tracklist.xml" );
	
	if ( !opmlFile.exists() )
		WriteXML();

	/*
    if ( !opmlFile.open( IO_ReadOnly ) ) 
        QMessageBox::critical( 0,
                tr( "Critical Error" ),
                tr( "Cannot open file %1" ).arg( m_sDirectory + "/tracklist.xml" ) );
        return;
    } 
	*/

	QDomDocument domXML( "Mixxx_Track_List" );
    if ( !domXML.setContent( &opmlFile ) ) {
        QMessageBox::critical( 0,
                tr( "Critical Error" ),
                tr( "Parsing error for file %1" ).arg( m_sDirectory + "/tracklist.xml" ) );
        opmlFile.close();
        return;
    }
    opmlFile.close();

    // Get all the tracks written in the xml file:
    QDomElement elementRoot = domXML.documentElement();
    QDomNode node = elementRoot.firstChild();
    while ( !node.isNull() ) {
        if ( node.isElement() && node.nodeName() == "Track" ) {
			// Create a new track:
			TrackInfoObject *Track;
			Track = new TrackInfoObject( node );
			// Append it to the list of tracks:
			m_lTracks.append( Track );
			qDebug( "Read track from xml file: %s", Track->m_sFilename.latin1() );
        }
        node = node.nextSibling();
    }

	// Run through all the files and add the new ones to the xml file:
	if (AddFiles( m_sDirectory ))
		WriteXML();
}

TrackList::~TrackList()
{
	// Delete all the tracks:
}

/*
	Write the xml tree to the file:
*/
void TrackList::WriteXML()
{
	// Create the xml document:
	QDomDocument domXML( "Mixxx_Track_List" );
	QDomElement elementRoot = domXML.createElement( "Mixxx_Track_List" );
	domXML.appendChild( elementRoot );

	// Insert all the tracks:

	for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next() )
	{
		QDomElement elementNew = domXML.createElement("Track");
		Track->WriteToXML( domXML, elementNew );
		elementRoot.appendChild( elementNew );
	}

	// Open the file:
	QFile opmlFile( m_sDirectory + "/tracklist.xml" );

    if ( !opmlFile.open( IO_WriteOnly) ) {
        QMessageBox::critical( 0,
                tr( "Critical Error" ),
                tr( "Cannot open file %1" ).arg( m_sDirectory + "/tracklist.xml" ) );
        return;
    }

	// Write to the file:
	QTextStream Xml( &opmlFile );
	Xml << domXML.toString();
	opmlFile.close();
}

/*
	Adds the files given in <path> to the list of files.
	Returns true if any new files were in fact added.
*/
bool TrackList::AddFiles(const char *path)
{    
	bool bFoundFiles = false;
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
            if ( AddFiles(d->filePath()) )
				bFoundFiles = true;
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
				// Append the track to the list of tracks:
				m_lTracks.append( Track );
				qDebug( "Found new track: %s", Track->m_sFilename.latin1() );
				bFoundFiles = true;
			}
            ++it;   // goto next list element
        }
	}
	return bFoundFiles;
}

TrackInfoObject *TrackList::FileExistsInList( const QString sFilename )
{	
	TrackInfoObject *Track;
	Track = m_lTracks.first();
	while ((Track) && (Track->m_sFilename != sFilename) )
		Track = m_lTracks.next();

	return Track;
}

