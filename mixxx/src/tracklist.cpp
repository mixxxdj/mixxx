#include <qstring.h>
#include <qdom.h>
#include <qptrlist.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qpopupmenu.h>
#include <qpoint.h>
#include <qtextcodec.h>
#include <qlabel.h>

#include "tracklist.h"
#include "trackinfoobject.h"
#ifdef __WIN__
  #include "soundsourcesndfile.h"
#endif
#ifdef __UNIX__
  #include "soundsourceaudiofile.h"
#endif
#include "soundsourcemp3.h"

#include "images/a.xpm"
#include "images/b.xpm"
#include "enginebuffer.h"
#include "dlgplaycontrol.h"
#include "reader.h"
#include "wtracktable.h"


TrackList::TrackList( const QString sDirectory, WTrackTable *ptableTracks,
                      DlgPlaycontrol *playcontrol1, DlgPlaycontrol *playcontrol2,
                      EngineBuffer *buffer1, EngineBuffer *buffer2)
{
    m_sDirectory = sDirectory;
    m_ptableTracks = ptableTracks;
    m_pPlaycontrol1 = playcontrol1;
    m_pPlaycontrol2 = playcontrol2;
    m_pBuffer1 = buffer1;
    m_pBuffer2 = buffer2;

    // Update the track list by reading the xml file, and adding new files:
    UpdateTracklist();

    // Construct popup menu used to select playback channel on track selection
    playSelectMenu = new QPopupMenu( );
    playSelectMenu->insertItem(QIconSet(a_xpm), "Player A",this, SLOT(slotChangePlay_1()));
    playSelectMenu->insertItem(QIconSet(b_xpm), "Player B",this, SLOT(slotChangePlay_2()));

    // Connect the right click to the slot where the menu is shown:
	connect( m_ptableTracks, SIGNAL( contextMenuRequested( int, int, const QPoint &) ),
		                     SLOT( slotRightClick( int, int, const QPoint &) ) );
}

TrackList::~TrackList()
{
	// Write out the xml file:
	WriteXML();

	// Delete all the tracks:
	for (unsigned int i=0; i<m_lTracks.count(); i++)
		delete m_lTracks.at(i);
}

/*
	Updates the score field (column 0) in the table.
*/
void TrackList::UpdateScores()
{
	for (unsigned int iRow=0; iRow<m_lTracks.count(); iRow++)
	{
		TrackInfoObject *track = m_lTracks.at( m_ptableTracks->text( iRow, COL_INDEX ).toInt() );
		m_ptableTracks->setText( iRow, COL_SCORE, 
		QString("%1").arg( (int) ( 99*track->m_iTimesPlayed/m_iMaxTimesPlayed ), 2 ) );
	}
}

/*
	Write the xml tree to the file:
*/
void TrackList::WriteXML()
{
    qDebug("Writing tracklist.xml");
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
#ifdef __WIN__
					SoundSourceSndFile::ParseHeader( Track );
#endif
#ifdef __UNIX__
					SoundSourceAudioFile::ParseHeader(Track);
#endif
                else if (sType == "mp3")
                    SoundSourceMp3::ParseHeader(Track);
                    
				// Append the track to the list of tracks:
				m_lTracks.append( Track );
//				qDebug( "Found new track: %s", Track->m_sFilename.latin1() );
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
    qDebug("Select track 1");
    TrackInfoObject *track = m_lTracks.at( 
		m_ptableTracks->text( m_ptableTracks->currentRow(), COL_INDEX ).toInt() );
        
	// Update score:
	track->m_iTimesPlayed++;
	if (track->m_iTimesPlayed > m_iMaxTimesPlayed)
		m_iMaxTimesPlayed = track->m_iTimesPlayed;
	UpdateScores();

	// Request a new track from the reader:
    m_pBuffer1->getReader()->requestNewTrack( track->Location() );
	
    // Write info
    m_pPlaycontrol1->textLabelTrack->setText( track->getInfo() );
}

void TrackList::slotChangePlay_2()
{
	TrackInfoObject *track = m_lTracks.at(		
		m_ptableTracks->text( m_ptableTracks->currentRow(), COL_INDEX ).toInt() );
        
	// Update score:
	track->m_iTimesPlayed++;
	if (track->m_iTimesPlayed > m_iMaxTimesPlayed)
		m_iMaxTimesPlayed = track->m_iTimesPlayed;
	UpdateScores();

	// Request a new track from the reader:
    m_pBuffer2->getReader()->requestNewTrack( track->Location() );
	
	// Write info
    m_pPlaycontrol2->textLabelTrack->
        setText( track->getInfo() );
}

/*
	Slot connected to popup menu activated when a track is clicked:
*/
void TrackList::slotRightClick( int iRow, int iCol, const QPoint &pos )
{
    qDebug("popup menu");
	// Display popup menu
    playSelectMenu->popup(pos);
}

void TrackList::UpdateTracklist()
{
    // Initialize xml file:
	QFile opmlFile( m_sDirectory + "/tracklist.xml" );
	
	if ( !opmlFile.exists() )
		WriteXML();

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
            if (!FileExistsInList( Track->m_sFilename ) )
            {
			    m_lTracks.append( Track );
//			    qDebug( "Read track from xml file: %s", Track->m_sFilename.latin1() );
            }
        }
        node = node.nextSibling();
    }

    // Run through all the files and add the new ones to the xml file:
	if (AddFiles( m_sDirectory ))
		WriteXML();

	// Put information from all the tracks into the table:
	int iRow=0;
	m_ptableTracks->hideColumn( COL_INDEX ); // Hide the index row
	m_ptableTracks->setNumRows( m_lTracks.count() );
	for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next() )
	{
		m_ptableTracks->setText( iRow, COL_TITLE, Track->m_sTitle );
		m_ptableTracks->setText( iRow, COL_ARTIST, Track->m_sArtist );
		m_ptableTracks->setText( iRow, COL_TYPE, Track->m_sType );
		m_ptableTracks->setText( iRow, COL_DURATION, Track->Duration() );
		m_ptableTracks->setText( iRow, COL_BITRATE, Track->m_sBitrate );
		m_ptableTracks->setText( iRow, COL_INDEX, QString("%1").arg(iRow) );
		iRow ++;
	}

	// Find the track which has been played the most times:
	m_iMaxTimesPlayed = 1;
	for (unsigned int i=0; i<m_lTracks.count(); i++)
		if ( m_lTracks.at(i)->m_iTimesPlayed > m_iMaxTimesPlayed)
			m_iMaxTimesPlayed = m_lTracks.at(i)->m_iTimesPlayed;

	// Update the scores for all the tracks:
	UpdateScores();
}
void TrackList::slotUpdateTracklist( QString sDir )
{
//    qDebug("dir: %s",sDir.latin1());

    // Save the "old" xml file:
    WriteXML();

    // Delete all "old" tracks:
    while (m_lTracks.count() != 0)
    {
        delete m_lTracks.first();
        m_lTracks.removeFirst();
    }

    // Set the new directory:
    m_sDirectory = sDir;

    // Make the newlist:
    UpdateTracklist();
}

