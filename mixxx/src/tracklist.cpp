#include <qstring.h>
#include <qdom.h>
#include <qptrlist.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qtable.h>
#include <qpopupmenu.h>
#include <qpoint.h>

#include "tracklist.h"
#include "trackinfoobject.h"
#include "soundsourcesndfile.h"
#include "images/a.xpm"
#include "images/b.xpm"

TrackList::TrackList( const QString sDirectory, QTable *ptableTracks ) 
{
	m_sDirectory = sDirectory;
	m_ptableTracks = ptableTracks;
	m_ptrackCurrent = 0;

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

	// Put information from all the tracks into the table:
	int iRow=0;
	m_ptableTracks->setNumRows( m_lTracks.count() );
	for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next() )
	{
		m_ptableTracks->setText( iRow, 1, Track->m_sTitle );
		m_ptableTracks->setText( iRow, 2, Track->m_sArtist );
		m_ptableTracks->setText( iRow, 3, Track->m_sType );
		m_ptableTracks->setText( iRow, 4, Track->Duration() );
		m_ptableTracks->setText( iRow, 5, Track->m_sBitrate );
		iRow ++;
	}

	// Find the track which has been played the most times:
	m_iMaxTimesPlayed = 1;
	for (unsigned int i=0; i<m_lTracks.count(); i++)
		if ( m_lTracks.at(i)->m_iTimesPlayed > m_iMaxTimesPlayed)
			m_iMaxTimesPlayed = m_lTracks.at(i)->m_iTimesPlayed;

	// Update the scores for all the tracks:
	UpdateScores();

	// Construct popup menu used to select playback channel on track selection
	playSelectMenu = new QPopupMenu( );
	playSelectMenu->insertItem(QIconSet(a_xpm), "Player A",this, SLOT(slotChangePlay_1()));
	playSelectMenu->insertItem(QIconSet(b_xpm), "Player B",this, SLOT(slotChangePlay_2()));

	// Connect the right click to the slot where the menu is shown:
	if (connect( m_ptableTracks, SIGNAL( pressed( int, int, int, const QPoint &) ),
		SLOT( slotRightClick( int, int, int, const QPoint &) ) ) )
		qDebug("Connected context menu on tracklist.");
}

TrackList::~TrackList()
{
	// Delete all the tracks:
}

void TrackList::UpdateScores()
{
	for (unsigned int i=0; i<m_lTracks.count(); i++)
		m_ptableTracks->setText( i, 0, 
		QString("%1").arg( (int) ( 99*m_lTracks.at(i)->m_iTimesPlayed/m_iMaxTimesPlayed ), 2 ) );
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
				TrackInfoObject *Track = 
					new TrackInfoObject( dir.absPath(), fi->fileName() );
				// Add basic information:
				Track->Parse(); 
				// Find the type:
				QString sType = fi->fileName().section(".",-1).lower();
				// Parse it using the sound sources:
				if (sType == "wav")
					SoundSourceSndFile::ParseHeader( Track );
				
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

/*
	Returns the TrackInfoObject which has the filename sFilename.
*/
TrackInfoObject *TrackList::FileExistsInList( const QString sFilename )
{	
	TrackInfoObject *Track;
	Track = m_lTracks.first();
	while ((Track) && (Track->m_sFilename != sFilename) )
		Track = m_lTracks.next();

	return Track;
}

/*
	These two slots basically just routes information further, but adds
	track information.
*/
void TrackList::slotChangePlay_1()
{
	TrackInfoObject *track = m_lTracks.at( m_ptableTracks->currentRow() );
	emit signalChangePlay_1( track );

	// Update score:
	track->m_iTimesPlayed++;
	UpdateScores();
}

void TrackList::slotChangePlay_2()
{
	TrackInfoObject *track = m_lTracks.at( m_ptableTracks->currentRow() );
	emit signalChangePlay_2( track );

	// Update score:
	track->m_iTimesPlayed++;
	UpdateScores();
}

/*
	Slot connected to popup menu activated when a track is clicked:
*/
void TrackList::slotRightClick( int iRow, int iCol, int iButton, const QPoint &pos )
{
	// Store the selected track:
	m_ptrackCurrent = m_lTracks.at( iRow );

	// Display popup menu
    playSelectMenu->popup(pos);
}

