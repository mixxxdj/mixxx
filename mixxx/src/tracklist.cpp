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
#include "enginebuffer.h"
#include "reader.h"
#include "readerextractbeat.h"
#include "wtracktable.h"
#include "wtracktableitem.h"
#include "wnumberpos.h"
#include "controlobject.h"
#include "enginebuffer.h"

TrackList::TrackList( const QString sDirectory, WTrackTable *pTableTracks,
                      QLabel *text1, QLabel *text2,
                      WNumberPos *pNumberPos1, WNumberPos *pNumberPos2,
                      EngineBuffer *buffer1, EngineBuffer *buffer2)
{
    m_sDirectory = sDirectory;
    m_pTableTracks = pTableTracks;
    m_pText1 = text1;
    m_pText2 = text2;
    m_pNumberPos1 = pNumberPos1;
    m_pNumberPos2 = pNumberPos2;
    m_pBuffer1 = buffer1;
    m_pBuffer2 = buffer2;

    m_iCurTrackIdxCh1 = -1;
    m_iCurTrackIdxCh2 = -1;
    m_pTrack1 = 0;
    m_pTrack2 = 0;
    m_lTracks.setAutoDelete(false);

    // Get ControlObject for determining end of track mode, and set default value to STOP.
    m_pEndOfTrackModeCh1 = ControlObject::getControl(ConfigKey("[Channel1]","TrackEndMode"));
    m_pEndOfTrackModeCh2 = ControlObject::getControl(ConfigKey("[Channel2]","TrackEndMode"));

    // Get pointers to ControlObjects for play buttons
    m_pPlayCh1 = ControlObject::getControl(ConfigKey("[Channel1]","play"));
    m_pPlayCh2 = ControlObject::getControl(ConfigKey("[Channel2]","play"));

    // Connect end-of-track signals to this object
    m_pEndOfTrackCh1 = ControlObject::getControl(ConfigKey("[Channel1]","TrackEnd"));
    m_pEndOfTrackCh2 = ControlObject::getControl(ConfigKey("[Channel2]","TrackEnd"));
    connect(m_pEndOfTrackCh1, SIGNAL(signalUpdateApp(double)), this, SLOT(slotEndOfTrackCh1(double)));
    connect(m_pEndOfTrackCh2, SIGNAL(signalUpdateApp(double)), this, SLOT(slotEndOfTrackCh2(double)));

    // Update the track list by reading the xml file, and adding new files:
    updateTracklist();

    // Construct popup menu used to select playback channel on track selection
    playSelectMenu = new QPopupMenu( );
    playSelectMenu->insertItem("Player 1",this, SLOT(slotChangePlay_1()));
    playSelectMenu->insertItem("Player 2",this, SLOT(slotChangePlay_2()));

    // Connect the right click to the slot where the menu is shown:
    connect(m_pTableTracks, SIGNAL(pressed(int, int, int, const QPoint &)),
                            SLOT(slotClick(int, int, int, const QPoint &)));
}

TrackList::~TrackList()
{
    // Write out the xml file:
    writeXML();

    // Delete all the tracks:
    for (unsigned int i=0; i<m_lTracks.count(); i++)
        delete m_lTracks.at(i);
}

void TrackList::updateTracklist()
{
    // Initialize xml file:
    QFile opmlFile(m_sDirectory + "/tracklist.xml");

    if (!opmlFile.exists())
        writeXML();

    QDomDocument domXML("Mixxx_Track_List");
    if (!domXML.setContent( &opmlFile))
    {
        QMessageBox::critical( 0,
                tr( "Critical Error" ),
                tr( "Parsing error for file %1" ).arg( m_sDirectory + "/tracklist.xml" ) );
        opmlFile.close();

        // Try writing a new file:
        writeXML();
    }
    opmlFile.close();

    // Get the version information:
    QDomElement elementRoot = domXML.documentElement();
    int iVersion = 0;
    QDomNode nodeVersion = TrackInfoObject::selectNode( elementRoot, "Version" );
    if (!nodeVersion.isNull() )
        iVersion = nodeVersion.toElement().text().toInt();

    // Get all the tracks written in the xml file:
    QDomNode node = elementRoot.firstChild();
    while ( !node.isNull() )
    {
        if ( node.isElement() && node.nodeName() == "Track" )
        {
            // Create a new track:
            TrackInfoObject *Track;
            Track = new TrackInfoObject(node);
            // Append it to the list of tracks:
            if (!fileExistsInList(Track->getFilename()))
            {
                // Shall we re-parse the header?:
                if (iVersion < TRACKLIST_VERSION)
                {
                    qDebug("Reparsed %s", Track->getFilename().latin1());
                    Track->parse();
                }
                m_lTracks.append(Track);
            }
        }
        node = node.nextSibling();
    }

    // Run through all the files and add the new ones to the xml file:
    bool bFilesAdded = addFiles(m_sDirectory);

    // Put information from all the tracks into the table:
    int iRow=0;
    int iTrackNo=0;
    m_pTableTracks->setNumRows(m_lTracks.count());
    for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next() )
    {
        if (Track->exists())
        {
            Track->insertInTrackTableRow(m_pTableTracks, iRow, iTrackNo);
            iRow ++;
        }
        iTrackNo ++;
    }
    // Readjust the number of rows:
    m_pTableTracks->setNumRows( iRow );

	// Find the track which has been played the most times:
	m_iMaxTimesPlayed = 1;
	for (unsigned int i=0; i<m_lTracks.count(); i++)
		if ( m_lTracks.at(i)->getTimesPlayed() > m_iMaxTimesPlayed)
			m_iMaxTimesPlayed = m_lTracks.at(i)->getTimesPlayed();

	// Update the scores for all the tracks:
	updateScores();

    if (bFilesAdded)
        writeXML();
}

void TrackList::slotEndOfTrackCh1(double)
{
//    qDebug("end of track ch 1");
    switch ((int)m_pEndOfTrackModeCh1->getValue())
    {
    case TRACK_END_MODE_NEXT:
        // Load next track
        m_iCurTrackIdxCh1++;
        slotChangePlay_1(m_iCurTrackIdxCh1);
        break;
/*
    case TRACK_END_MODE_STOP:
        //m_pPlayCh1->setValueFromApp(0.);
        break;
    case TRACK_END_MODE_LOOP:
        // Load same track
        slotChangePlay_1(m_iCurTrackIdxCh1);
        break;
    case TRACK_END_MODE_PING:
        qDebug("EndOfTrack mode ping not yet implemented");
        break;
*/
    default:
        qDebug("Invalid EndOfTrack mode value");
    }
    m_pEndOfTrackCh1->setValueFromApp(0.);
}

void TrackList::slotEndOfTrackCh2(double)
{
    switch ((int)m_pEndOfTrackModeCh2->getValue())
    {
    case TRACK_END_MODE_NEXT:
        // Load next track
        m_iCurTrackIdxCh2++;
        slotChangePlay_2(m_iCurTrackIdxCh2);
        break;
/*
    case TRACK_END_MODE_STOP:
        //m_pPlayCh2->setValueFromApp(0.);
        break;
    case TRACK_END_MODE_LOOP:
        // Load same track
        slotChangePlay_2(m_iCurTrackIdxCh2);
        break;
    case TRACK_END_MODE_PING:
        qDebug("EndOfTrack mode ping not yet implemented");
        break;
*/
    default:
        qDebug("Invalid EndOfTrack mode value");
    }
    m_pEndOfTrackCh2->setValueFromApp(0.);
}

/*
    Updates the score field (column 0) in the table.
*/
void TrackList::updateScores()
{
    for (unsigned int iRow=0; iRow<m_lTracks.count(); iRow++)
    {
        TrackInfoObject *pTrack = m_lTracks.at(m_pTableTracks->text(iRow, COL_INDEX).toInt());
        pTrack->setScore(99*pTrack->getTimesPlayed()/m_iMaxTimesPlayed);
    }
}

/*
	Write the xml tree to the file:
*/
void TrackList::writeXML()
{
    qDebug("Writing %stracklist.xml, %d tracks", m_sDirectory.latin1(),m_pTableTracks->numRows());
    // First transfer information from the comment field from the table to the Track:
    for (unsigned int iRow=0; iRow<m_pTableTracks->numRows(); iRow++)
    {
        if (m_pTableTracks->item(iRow, COL_INDEX))
        {
            m_lTracks.at( m_pTableTracks->item(iRow, COL_INDEX)->text().toUInt() )->setComment(m_pTableTracks->item(iRow, COL_COMMENT)->text());
        }
    }

    // Create the xml document:
    QDomDocument domXML( "Mixxx_Track_List" );

    // Ensure UTF16 encoding
    domXML.appendChild(domXML.createProcessingInstruction("xml","version=\" VERSION \" encoding=\"UTF-16\""));

    // Set the document type
    QDomElement elementRoot = domXML.createElement( "Mixxx_Track_List" );
    domXML.appendChild(elementRoot);

    // Add version information:
    TrackInfoObject::addElement( domXML, elementRoot, "Version", QString("%1").arg( TRACKLIST_VERSION ) );

    // Insert all the tracks:
    for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next())
    {
        QDomElement elementNew = domXML.createElement("Track");
        // See if we should add information from the comment field:
        Track->writeToXML(domXML, elementNew);
        elementRoot.appendChild(elementNew);
    }

    // Open the file:
    QFile opmlFile(m_sDirectory + "/tracklist.xml");
    if (!opmlFile.open(IO_WriteOnly))
    {
        QMessageBox::critical(0,
                tr("Error"),
                tr("Cannot open file %1").arg(m_sDirectory + "/tracklist.xml"));
        return;
    }

    // Write to the file:
    QTextStream Xml(&opmlFile);
    Xml.setEncoding(QTextStream::Unicode);
    Xml << domXML.toString();
    opmlFile.close();
}

bool TrackList::addFiles(const char *path)
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
        while ((d=dir_it.current()))
        {
        if (!d->filePath().endsWith(".") && !d->filePath().endsWith(".."))
            {
                if (addFiles(d->filePath()))
                    bFoundFiles = true;
            }
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
            //qDebug("filename %s",fi->fileName().latin1());

            // Check if the file exists in the list:
            TrackInfoObject *Track = fileExistsInList(fi->fileName());
            if (!Track)
            {
                Track = new TrackInfoObject( dir.absPath(), fi->fileName() );

                // Append the track to the list of tracks:
                if (Track->parse() == OK)
                {
                    m_lTracks.append(Track);
                    qDebug( "Found new track: %s", Track->getFilename().latin1() );
                    bFoundFiles = true;
                }
                else
                    qWarning("Could not parse %s", fi->fileName().latin1());
            }
            else
            // If it exists in the list already, it might not have been found in the
            // first place because it has been moved:
            if (!Track->exists())
            {
                Track->setFilepath(fi->dirPath());
                Track->checkFileExists();
                if (Track->exists())
                    qDebug("Refound %s", Track->getFilename().latin1());
            }
            ++it;   // goto next list element
        }
    }
    return bFoundFiles;
}

/*
    Returns the TrackInfoObject which has the filename sFilename.
*/
TrackInfoObject *TrackList::fileExistsInList(const QString sFilename)
{
    TrackInfoObject *pTrack;
    pTrack = m_lTracks.first();
    while ((pTrack) && (pTrack->getFilename() != sFilename))
        pTrack = m_lTracks.next();

    return pTrack;
}

/*
    These two slots basically just routes information further, but adds
    track information.
*/
void TrackList::slotChangePlay_1(int idx)
{
    if (idx==-1)
        m_iCurTrackIdxCh1 = m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX ).toInt();
    m_pTrack1 = m_lTracks.at(m_iCurTrackIdxCh1);

    if (m_pTrack1)
    {
        // Update score:
        m_pTrack1->incTimesPlayed();
        if (m_pTrack1->getTimesPlayed() > m_iMaxTimesPlayed)
            m_iMaxTimesPlayed = m_pTrack1->getTimesPlayed();
        updateScores();

        // Request a new track from the reader:
        m_pBuffer1->getReader()->requestNewTrack(m_pTrack1);

        // Write info
        m_pText1->setText(m_pTrack1->getInfo());

        // Set duration in playpos widget
        m_pNumberPos1->setDuration(m_pTrack1->getDuration());
    }
}

void TrackList::slotChangePlay_2(int idx)
{
    if (idx==-1)
        m_iCurTrackIdxCh2 = m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX).toInt();
    m_pTrack2 = m_lTracks.at(m_iCurTrackIdxCh2);

    if (m_pTrack2)
    {
        // Update score:
        m_pTrack2->incTimesPlayed();
        if (m_pTrack2->getTimesPlayed() > m_iMaxTimesPlayed)
            m_iMaxTimesPlayed = m_pTrack2->getTimesPlayed();
        updateScores();

        // Request a new track from the reader:
        m_pBuffer2->getReader()->requestNewTrack(m_pTrack2);

        // Write info
        m_pText2->setText( m_pTrack2->getInfo() );

        // Set duration in playpos widget
        m_pNumberPos2->setDuration(m_pTrack2->getDuration());
    }
}

void TrackList::loadTrack1(QString name)
{
/*
    if (QFile(name).exists())
    {
        m_pBuffer1->getReader()->requestNewTrack(name);
        m_pText1->setText(name);
    }
*/
}
void TrackList::loadTrack2(QString name)
{
/*
    if (QFile(name).exists())
    {
        m_pBuffer2->getReader()->requestNewTrack(name);
        m_pText2->setText(name);
    }
*/
}

/*
    Slot connected to popup menu activated when a track is clicked:
*/
void TrackList::slotClick( int iRow, int iCol, int iButton, const QPoint &pos )
{
    // Display popup menu if mouse pointer is placed outside comment row
    if (pos.x()<m_pTableTracks->columnPos(COL_COMMENT) ||
        pos.x()>m_pTableTracks->columnPos(COL_COMMENT)+m_pTableTracks->columnWidth(COL_COMMENT))
    {
        QPoint globalPos = m_pTableTracks->mapToGlobal(pos);
        globalPos -= QPoint(m_pTableTracks->contentsX(), m_pTableTracks->contentsY());
        playSelectMenu->popup(globalPos);
    }
}


void TrackList::slotUpdateTracklist( QString sDir )
{
//    qDebug("dir: %s",sDir.latin1());

    // Save the "old" xml file:
    writeXML();

    // Delete all "old" tracks:
    while (m_lTracks.count() != 0)
    {
        // Do not delete the TrackInfoObject from memory if it is currently played. The Reader may write
        // to it when unloading the track!
        if (m_pTrack1!=m_lTracks.first() && m_pTrack2!=m_lTracks.first())
            delete m_lTracks.first();
        else
            m_lTracks.first()->removeFromTrackTable();
        m_lTracks.removeFirst();
    }

    // Delete contents of tabletrack
    m_pTableTracks->setNumRows(0);

    // Set the new directory:
    m_sDirectory = sDir;

    // Make the newlist:
    updateTracklist();
}

