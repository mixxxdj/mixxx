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

#include "tracklist.h"
#include "trackinfoobject.h"
#ifdef __WIN__
  #include "soundsourcesndfile.h"
#endif
#ifdef __UNIX__
  #include "soundsourceaudiofile.h"
#endif
#include "soundsourcemp3.h"
#include "soundsourceoggvorbis.h"

#include "images/a.xpm"
#include "images/b.xpm"
#include "enginebuffer.h"
#include "reader.h"
#include "wtracktable.h"
#include "wtracktableitem.h"
#include "controlobject.h"

TrackList::TrackList( const QString sDirectory, WTrackTable *ptableTracks,
                      QLabel *text1, QLabel *text2,
                      EngineBuffer *buffer1, EngineBuffer *buffer2)
{
    m_sDirectory = sDirectory;
    m_ptableTracks = ptableTracks;
    m_pText1 = text1;
    m_pText2 = text2;
    m_pBuffer1 = buffer1;
    m_pBuffer2 = buffer2;

    m_iCurTrackIdxCh1 = -1;
    m_iCurTrackIdxCh2 = -1;

    // Construct controlpotmeter for determining end of track mode, and set default value to STOP.
    m_pEndOfTrackModeCh1 = new ControlObject(ConfigKey("[Channel1]","EndOfTrackMode"));
    m_pEndOfTrackModeCh2 = new ControlObject(ConfigKey("[Channel2]","EndOfTrackMode"));   
    m_pEndOfTrackModeCh1->setValueFromApp((double)END_OF_TRACK_MODE_STOP);
    m_pEndOfTrackModeCh2->setValueFromApp((double)END_OF_TRACK_MODE_STOP);

    // Get pointers to ControlObjects for play buttons
    ControlObject *c = m_pEndOfTrackModeCh1;
    m_pPlayCh1 = c->getControl(ConfigKey("[Channel1]","play"));
    m_pPlayCh2 = c->getControl(ConfigKey("[Channel2]","play"));

    // Connect end-of-track signals to this object
    m_pEndOfTrackCh1 = c->getControl(ConfigKey("[Channel1]","EndOfTrack"));
    connect(m_pEndOfTrackCh1, SIGNAL(signalUpdateApp(double)), this, SLOT(slotEndOfTrackCh1(double)));
    m_pEndOfTrackCh2 = c->getControl(ConfigKey("[Channel2]","EndOfTrack"));
    connect(m_pEndOfTrackCh2, SIGNAL(signalUpdateApp(double)), this, SLOT(slotEndOfTrackCh2(double)));    
    
    // Update the track list by reading the xml file, and adding new files:
    UpdateTracklist();

    // Construct popup menu used to select playback channel on track selection
    playSelectMenu = new QPopupMenu( );
    playSelectMenu->insertItem(QIconSet(a_xpm), "Player A",this, SLOT(slotChangePlay_1()));
    playSelectMenu->insertItem(QIconSet(b_xpm), "Player B",this, SLOT(slotChangePlay_2()));

    // Connect the right click to the slot where the menu is shown:
    connect(m_ptableTracks, SIGNAL(pressed(int, int, int, const QPoint &)),
                            SLOT(slotClick(int, int, int, const QPoint &)));

}

TrackList::~TrackList()
{
	// Write out the xml file:
	WriteXML();

	// Delete all the tracks:
	for (unsigned int i=0; i<m_lTracks.count(); i++)
		delete m_lTracks.at(i);
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

    // Get the version information:
    QDomElement elementRoot = domXML.documentElement();
    int iVersion = 0;
    QDomNode nodeVersion = TrackInfoObject::SelectNode( elementRoot, "Version" );
    if (!nodeVersion.isNull() )
        iVersion = nodeVersion.toElement().text().toInt();

    // Get all the tracks written in the xml file:
    QDomNode node = elementRoot.firstChild();
    while ( !node.isNull() ) {
        if ( node.isElement() && node.nodeName() == "Track" ) {
			// Create a new track:
			TrackInfoObject *Track;
			Track = new TrackInfoObject( node );
			// Append it to the list of tracks:
            if (!FileExistsInList( Track->m_sFilename ) )
            {
                // Shall we re-parse the header?:
                if (iVersion < TRACKLIST_VERSION)
                {
                    qDebug("Reparsed %s", Track->m_sFilename.latin1() );
                    if (ParseHeader( Track ) == OK) 
			            m_lTracks.append( Track );
                }
                else
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
        m_ptableTracks->setItem(iRow, COL_TITLE, new WTrackTableItem(m_ptableTracks,QTableItem::Never, Track->m_sTitle));
		m_ptableTracks->setItem(iRow, COL_ARTIST, new WTrackTableItem(m_ptableTracks,QTableItem::Never, Track->m_sArtist));
		m_ptableTracks->setItem(iRow, COL_TYPE, new WTrackTableItem(m_ptableTracks,QTableItem::Never, Track->m_sType));
		m_ptableTracks->setItem(iRow, COL_DURATION, new WTrackTableItem(m_ptableTracks,QTableItem::Never, Track->Duration()));
		m_ptableTracks->setItem(iRow, COL_BITRATE, new WTrackTableItem(m_ptableTracks,QTableItem::Never, Track->m_sBitrate));
		m_ptableTracks->setItem(iRow, COL_INDEX, new WTrackTableItem(m_ptableTracks,QTableItem::Never, QString("%1").arg(iRow)));
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

void TrackList::slotEndOfTrackCh1(double)
{
    switch ((int)m_pEndOfTrackModeCh1->getValue())
    {
    case END_OF_TRACK_MODE_STOP:
        m_pPlayCh1->setValueFromApp(0.);
        break;
    case END_OF_TRACK_MODE_NEXT:
        // Load next track
        m_iCurTrackIdxCh1++;
        slotChangePlay_1(m_iCurTrackIdxCh1);
        break;
    case END_OF_TRACK_MODE_LOOP:
        // Load same track
        slotChangePlay_1(m_iCurTrackIdxCh1);
        break;
    case END_OF_TRACK_MODE_PING:
        qDebug("EndOfTrack mode ping not yet implemented");
        break;
    default:
        qDebug("Invalid EndOfTrack mode value");
    }
    m_pEndOfTrackCh1->setValueFromApp(0.);
}

void TrackList::slotEndOfTrackCh2(double)
{
    switch ((int)m_pEndOfTrackModeCh2->getValue())
    {
    case END_OF_TRACK_MODE_STOP:
        m_pPlayCh2->setValueFromApp(0.);
        break;
    case END_OF_TRACK_MODE_NEXT:
        // Load next track
        m_iCurTrackIdxCh2++;
        slotChangePlay_2(m_iCurTrackIdxCh2);
        break;
    case END_OF_TRACK_MODE_LOOP:
        // Load same track
        slotChangePlay_2(m_iCurTrackIdxCh2);
        break;
    case END_OF_TRACK_MODE_PING:
        qDebug("EndOfTrack mode ping not yet implemented");
        break;
    default:
        qDebug("Invalid EndOfTrack mode value");
    }
    m_pEndOfTrackCh2->setValueFromApp(0.);
}

/*
    Updates the score field (column 0) in the table.
*/
void TrackList::UpdateScores()
{
	for (unsigned int iRow=0; iRow<m_lTracks.count(); iRow++)
	{
		TrackInfoObject *track = m_lTracks.at( m_ptableTracks->text( iRow, COL_INDEX ).toInt() );
        m_ptableTracks->setItem(iRow, COL_SCORE, new WTrackTableItem(m_ptableTracks,QTableItem::Never,
                                QString("%1").arg( (int) ( 99*track->m_iTimesPlayed/m_iMaxTimesPlayed ), 2 ) ));
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

    // Add version information:
    TrackInfoObject::AddElement( domXML, elementRoot, "Version", QString("%1").arg( TRACKLIST_VERSION ) );

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
                tr( "Error" ),
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
        dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3 *.ogg *.Ogg *.OGG");
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
                
                    // Append the track to the list of tracks:
                    if (ParseHeader( Track ) == OK) {
                        m_lTracks.append( Track );
                        //qDebug( "Found new track: %s", Track->m_sFilename.latin1() );
                        bFoundFiles = true;
                    } 
                    else
                        qWarning("Could not parse %s", fi->fileName().latin1());

			}
            ++it;   // goto next list element
        }
	}
	return bFoundFiles;
}

/*
    Fill in the information in the track, by using the static member ParseHeader
    in the different SoundSource objects.
*/
int TrackList::ParseHeader( TrackInfoObject *Track )
{
    // Add basic information:
	Track->Parse(); 
	// Find the type:
	QString sType = Track->m_sFilename.section(".",-1).lower();
	// Parse it using the sound sources:
    int iResult = ERR;
	if (sType == "wav")
#ifdef __WIN__
     iResult = SoundSourceSndFile::ParseHeader( Track );
#endif
#ifdef __UNIX__
    iResult = SoundSourceAudioFile::ParseHeader(Track);
#endif
     else if (sType == "mp3")
        iResult = SoundSourceMp3::ParseHeader(Track);
    else if (sType == "ogg")
        iResult = SoundSourceOggVorbis::ParseHeader(Track);

    // Try to sort out obviously erroneous parsings:
    int iBitrate = Track->m_sBitrate.toInt();
    if ((iBitrate <= 0) || (iBitrate > 5000))
        Track->m_sBitrate = "?";
                    
    return iResult;
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
void TrackList::slotChangePlay_1(int idx)
{
    if (idx==-1)
        m_iCurTrackIdxCh1 = m_ptableTracks->text(m_ptableTracks->currentRow(), COL_INDEX ).toInt();
    TrackInfoObject *track = m_lTracks.at(m_iCurTrackIdxCh1);
    
    // Update score:
    track->m_iTimesPlayed++;
    if (track->m_iTimesPlayed > m_iMaxTimesPlayed)
        m_iMaxTimesPlayed = track->m_iTimesPlayed;
    UpdateScores();

    // Request a new track from the reader:
    m_pBuffer1->getReader()->requestNewTrack( track->Location() );

    // Write info
    m_pText1->setText( track->getInfo() );
}

void TrackList::slotChangePlay_2(int idx)
{
    if (idx==-1)
        m_iCurTrackIdxCh2 = m_ptableTracks->text(m_ptableTracks->currentRow(), COL_INDEX).toInt();
    TrackInfoObject *track = m_lTracks.at(m_iCurTrackIdxCh2);
        
    // Update score:
    track->m_iTimesPlayed++;
    if (track->m_iTimesPlayed > m_iMaxTimesPlayed)
        m_iMaxTimesPlayed = track->m_iTimesPlayed;
    UpdateScores();

    // Request a new track from the reader:
    m_pBuffer2->getReader()->requestNewTrack( track->Location() );
    
    // Write info
    m_pText2->setText( track->getInfo() );
}

/*
    Slot connected to popup menu activated when a track is clicked:
*/
void TrackList::slotClick( int iRow, int iCol, int iButton, const QPoint &pos )
{
    // Display popup menu
    QPoint globalPos = m_ptableTracks->mapToGlobal(pos);
    globalPos -= QPoint(m_ptableTracks->contentsX(), m_ptableTracks->contentsY());
    playSelectMenu->popup(globalPos);
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

