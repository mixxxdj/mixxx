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
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qregexp.h>

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

TrackList::TrackList( const QString sDirectory,const QString sPlaylistdir ,WTrackTable *pTableTracks, WTreeList *pTree,
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

    wTree = pTree;
    wTree->m_sPlaylistdir = sPlaylistdir;
    wTree->setRoot(m_sDirectory);
    wTree->setPlaylist(wTree->m_sPlaylistdir);

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

    // Set default playlist
    currentPlaylist = "default.xml";

    treeSelectMenu = new QPopupMenu();
    treeSelectMenu->insertItem("Delete Playlist",this,SLOT(slotDeletePlaylist()));
    treeSelectMenu->insertItem("Search for Trackname",this,SLOT(slotFindTrack()));

	// Construct popup menu used to select playback channel on track selection
	/**
	NEW!!! : Deleting Tracks, Save current Playlist , save playlist as
	**/
    playSelectMenu = new QPopupMenu( );
    playSelectMenu->insertItem("Player 1",this, SLOT(slotChangePlay_1()));
    playSelectMenu->insertItem("Player 2",this, SLOT(slotChangePlay_2()));
	//playSelectMenu->insertItem("--------",this);
	playSelectMenu->insertItem("Save Playlist", this, SLOT(writeXML()));
	playSelectMenu->insertItem("Save Playlist as", this, SLOT(slotBrowseDir()));
	playSelectMenu->insertItem("Delete Track",this, SLOT(slotDeleteTrack()));
	playSelectMenu->insertItem("Clear Playlist", this, SLOT(slotClearPlaylist()));
	playSelectMenu->insertSeparator(2);
	playSelectMenu->insertSeparator(5);

    // Connect the right click to the slot where the menu is shown:
    connect(m_pTableTracks, SIGNAL(pressed(int, int, int, const QPoint &)),
                            SLOT(slotClick(int, int, int, const QPoint &)));
	connect(wTree, SIGNAL(rightButtonPressed ( QListViewItem *, const QPoint &, int )),
                   SLOT(slotTreeClick(QListViewItem *, const QPoint &, int)));

	//connect applyDir signal with the UpdateTracklist slot, as described in wTreeList
	connect(m_pTableTracks, SIGNAL(applyDir(QString )), this, SLOT(slotUpdateTracklist( QString  )));
	connect(wTree, SIGNAL(loadPls(QString)), this, SLOT(slotUpdateTracklistFake( QString) ) );
}

TrackList::~TrackList()
{
    qDebug("destroying tracklist");
    // Initialize xml file:
    QFile opmlFile(wTree->m_sPlaylistdir);
    QDomDocument domXML("Mixxx_Track_List");
    if (!domXML.setContent( &opmlFile))
    {
        QMessageBox::critical( 0,
                tr( "Critical Error" ),
                tr( "Parsing error for file %1" ).arg( wTree->m_sPlaylistdir.latin1() ) );
        opmlFile.close();

        // Try writing a new file:
        //writeXML();
    }
    opmlFile.close();

    // Get the version information:
    QDomElement elementRoot = domXML.documentElement();
/*
    int iVersion = 0;
    QDomNode nodeVersion = TrackInfoObject::selectNode( elementRoot, "Version" );
    if (!nodeVersion.isNull() )
        iVersion = nodeVersion.toElement().text().toInt();
*/

    // Get all the Playlists written in the xml file:
    QDomNode node = elementRoot.firstChild();
    // Only write out the xml file if the current playlist is still contained in it(wasnt deleted beforehand but is still flagged aktive):
    while ( !node.isNull() )
    {
        if(node.isElement() && node.nodeName() == "Playlist" &&
           node.toElement().attribute( "Name" ) == currentPlaylist)
        {
            writeXML();
            break;
/*
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
*/
        }
        node = node.nextSibling();
    }

    // Delete all the tracks:
    for (unsigned int i=0; i<m_lTracks.count(); i++)
        delete m_lTracks.at(i);
}

void TrackList::updateTracklist()
{
    // Run through all the files and add the new ones to the xml file:
	bool bFilesAdded = false;
	
	// Put information from all the tracks into the table:
	/************************************************************
	the following code has been added for debugging
	tracks should only be added to the table list if there are new ones
	in the m_lTracks Pointer list. This is being determined by AddFiles
	anyhow (checks if files are existing in the tracklist allready, if so
	they have to be existing as an Item in the tablelist by now)
	************************************************************/
	
	int iRow=0;
    if(!m_sDirectory.endsWith(".xml"))
    {
	    bFilesAdded = addFiles(m_sDirectory.latin1());
	    iRow= m_pTableTracks->numRows();
    }
    else
    {
		bFilesAdded = TRUE;
	}

	int iTrackNo=0;
	int iTrackCount=m_lTracks.count();

	if(!bFilesAdded && m_lTracks.count() == m_pTableTracks->numRows())
    {

		iRow = 0;
		iTrackCount = 0;
	}

    m_pTableTracks->insertRows(m_pTableTracks->numRows(), iTrackCount );
	for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next() )
    {
        if (Track->exists())
        {
            Track->insertInTrackTableRow(m_pTableTracks, iRow, iTrackNo);
            iRow ++;
        }
        iTrackNo ++;
    }

	qDebug("Number of rows written: %d",iRow);

	// Find the track which has been played the most times:
	m_iMaxTimesPlayed = 1;
	for (unsigned int i=0; i<m_lTracks.count(); i++)
		if ( m_lTracks.at(i)->getTimesPlayed() > m_iMaxTimesPlayed)
			m_iMaxTimesPlayed = m_lTracks.at(i)->getTimesPlayed();

	// Update the scores for all the tracks:
	updateScores();
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
    bool newPlaylist = true;
	bool newFile = false;
	QFile opmlFile(wTree->m_sPlaylistdir);

    // Create the xml document:
	QDomDocument domXML("Mixxx_Track_List");

    qDebug("PlaylistID is: %s", currentPlaylist.latin1());
	if (!opmlFile.exists()){

	qDebug("Could not open the default playlist file!");
	newFile = TRUE;
		}
    if (!domXML.setContent( &opmlFile))
    {

        opmlFile.close();

        //WriteXML();
    }

	opmlFile.close();
	qDebug("Writing ID %s, %d tracks", currentPlaylist.latin1(),m_pTableTracks->numRows());
    // First transfer information from the comment field from the table to the Track:
    for (int iRow=0; iRow<m_pTableTracks->numRows(); iRow++)
    {
        if (m_pTableTracks->item(iRow, COL_INDEX))
        {
            m_lTracks.at( m_pTableTracks->item(iRow, COL_INDEX)->text().toUInt() )->setComment(m_pTableTracks->item(iRow, COL_COMMENT)->text());
        }
    }

    // Ensure UTF16 encoding
    if(newFile)
    {
        domXML.appendChild(domXML.createProcessingInstruction("xml","version=\" VERSION \" encoding=\"UTF-16\""));

        // Set the document type
        QDomElement elementRoot = domXML.createElement( "Mixxx_Track_List" );
        domXML.appendChild(elementRoot);

        // Add version information:
        TrackInfoObject::addElement( domXML, elementRoot, "Version", QString("%1").arg( TRACKLIST_VERSION ) );
    }
    QDomElement elementRoot = domXML.documentElement();

    // Get all the tracks written in the xml file:
    QDomNode node = elementRoot.firstChild();
    while ( !node.isNull() )
    {
        if(node.isElement() && node.nodeName() == "Playlist" &&
           node.toElement().attribute( "Name" ) == currentPlaylist)
        {
            newPlaylist = false;
            //First delete all old childs
            while(node.hasChildNodes())
                node.removeChild(node.firstChild());

            // Insert all the tracks:
            for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next())
            {
                QDomElement elementNew = domXML.createElement("Track");

                // See if we should add information from the comment field:
                Track->writeToXML(domXML, elementNew);
                node.appendChild(elementNew);
            }
        }
        node = node.nextSibling();
    }

    if(newPlaylist)
    {
        QDomElement playlistElement = domXML.createElement( "Playlist" );
        playlistElement.setAttribute("Name",currentPlaylist);
        for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next())
        {
            QDomElement elementNew = domXML.createElement("Track");
            // See if we should add information from the comment field:
            Track->writeToXML(domXML, elementNew);
            playlistElement.appendChild(elementNew);
        }
        elementRoot.appendChild(playlistElement);
    }

    if (!opmlFile.open(IO_WriteOnly))
    {
        QMessageBox::critical(0,
                tr("Error"),
                tr("Cannot open file %1").arg(wTree->m_sPlaylistdir.latin1()));
        return;
    }

    // Write to the file:
    QTextStream Xml(&opmlFile);
    Xml.setEncoding(QTextStream::Unicode);
    Xml << domXML.toString();
    opmlFile.close();

    //Update the Treelist
    refreshPlaylist();
}

bool TrackList::addFiles(const char *path)
{
    bool bFoundFiles = false;
    // First run through all directories:
    QDir dir(path);

	if (!dir.exists()){
         QFile mFile(path);

	    if(!mFile.exists())
		   qDebug("Cannot find the path: %s",path);


	    else{
		QFileInfo m_fileInfo(mFile);

		int iTrackNumber = 0;
		if(!m_lTracks.isEmpty())
		iTrackNumber = m_lTracks.last()->m_iIndex+1;
			// Check if the file exists in the list:
            TrackInfoObject *Track;
			Track = fileExistsInList( m_fileInfo.fileName() );
			if (!Track)
            {
                Track = new TrackInfoObject( m_fileInfo.dir().absPath(), m_fileInfo.fileName() );
                Track->m_iIndex = iTrackNumber;

                // Append the track to the list of tracks:
                if (Track->parse() == OK)
                {
                    m_lTracks.append(Track);
                    qDebug( "Found new track: %s", Track->getFilename().latin1() );
                    bFoundFiles = true;
                } 
                else
                    qWarning("Could not parse %s", m_fileInfo.fileName().latin1());
            } 
            else
            // If it exists in the list already, it might not have been found in the
            // first place because it has been moved:
            if (!Track->exists())
            {
                Track->setFilepath(m_fileInfo.dirPath());
                Track->checkFileExists();
                qDebug("Refound %s", Track->getFilename().latin1() );
            }

	    }
	}
	else
    {
        dir.setFilter(QDir::Dirs);
        if (dir.entryInfoList()==0)
            return bFoundFiles;

        const QFileInfoList dir_list = *dir.entryInfoList();


        QFileInfoListIterator dir_it(dir_list);
        QFileInfo *d;
        while (d=dir_it.current())
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
		
		
		int iTrackNumber = 0;
		if(!m_lTracks.isEmpty())
		iTrackNumber = m_lTracks.last()->m_iIndex+1;
		
        while ((fi=it.current()))
        {
            //qDebug("filename %s",fi->fileName().latin1());

            // Check if the file exists in the list:
            TrackInfoObject *pTrack = fileExistsInList(fi->fileName());
            if (!pTrack)
            {
                pTrack = new TrackInfoObject( dir.absPath(), fi->fileName() );
                pTrack->m_iIndex = iTrackNumber;

                // Append the track to the list of tracks:
                if (pTrack->parse() == OK)
                {
                    m_lTracks.append(pTrack);
                    qDebug( "Found new track: %s", pTrack->getFilename().latin1() );
                    bFoundFiles = true;
                }
                else
                    qWarning("Could not parse %s", fi->fileName().latin1());
            }
            else if (!pTrack->exists())
            {
                // If it exists in the list already, it might not have been found in the
                // first place because it has been moved:
                pTrack->setFilepath(fi->dirPath());
                pTrack->checkFileExists();
                if (pTrack->exists())
                    qDebug("Refound %s", pTrack->getFilename().latin1());
            }
            ++it;   // goto next list element
        	++iTrackNumber;
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
//Refreshes the Tree of the Playlist Repository
void TrackList::refreshPlaylist(){

	wTree->setPlaylist(wTree->m_sPlaylistdir);
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
    if (QFile(name).exists())
    {
        TrackInfoObject *t = new TrackInfoObject("",name);
        m_pBuffer1->getReader()->requestNewTrack(t);
        m_pText1->setText(t->getInfo());
    }
}
void TrackList::loadTrack2(QString name)
{
    if (QFile(name).exists())
    {
        TrackInfoObject *t = new TrackInfoObject("",name);
        m_pBuffer2->getReader()->requestNewTrack(t);
        m_pText2->setText(t->getInfo());
    }
}
//Deletes a Track from the Table
void TrackList::slotDeleteTrack(int idx)
{

	if (idx==-1){
		 idx = m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX).toInt();
	}
		TrackInfoObject *Track = m_lTracks.at(idx);
	qDebug("Removing index at: %d", idx);
	m_lTracks.remove(idx);
	for(int i=idx; i<m_lTracks.count();i++)
		m_lTracks.at(i)->m_iIndex = i;

/*
    ******************* FIX ME:
    if (Track)
	{
		for(int i= 0; i<m_lTracks.count(); i++){
            //Iterate through the whole table from point idx on
			//int t_idx = m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX).toInt();
			//m_pTableTracks->setItem(i,0,m_pTableTracks->item(o,0));//Item gets replaced by Item below
			Track = m_lTracks.at(i);
			m_pTableTracks->setItem(i, COL_TITLE, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->m_sTitle, typeText));
    		m_pTableTracks->setItem(i, COL_ARTIST, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->m_sArtist, typeText));
	    	m_pTableTracks->setItem(i, COL_COMMENT, new WTrackTableItem(m_pTableTracks,QTableItem::WhenCurrent, Track->m_sComment, typeText));
		    m_pTableTracks->setItem(i, COL_TYPE, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->m_sType, typeText));
	    	m_pTableTracks->setItem(i, COL_DURATION, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->Duration(), typeDuration));
		    m_pTableTracks->setItem(i, COL_BITRATE, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->m_sBitrate, typeNumber));
    		m_pTableTracks->setItem(i, COL_INDEX, new WTrackTableItem(m_pTableTracks,QTableItem::Never, QString("%1").arg(Track->m_iIndex), typeText));

		    }
		}
		m_pTableTracks->setNumRows(m_lTracks.count());
	}
*/
}
/*
    Slot connected to popup menu activated when a track is clicked:
*/
void TrackList::slotClick(int, int, int, const QPoint &pos )
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
/** Can be called to Update the Root Directory in the TreeList **/
void TrackList::slotUpdateRoot(QString sDir)
{

	wTree->setRoot(sDir);


	}

void TrackList::loadPlaylist( QString sPlaylist ) {

	// Initialize xml file:
    QFile opmlFile(wTree->m_sPlaylistdir);
	currentPlaylist = sPlaylist;


    QDomDocument domXML("Mixxx_Track_List");
    if (!domXML.setContent( &opmlFile))
    {
        QMessageBox::critical( 0,
                tr( "Critical Error" ),
                tr( "Parsing error for file %1" ).arg( wTree->m_sPlaylistdir.latin1() ) );
        opmlFile.close();
        // Try writing a new file:
        //WriteXML();
    }
    opmlFile.close();

    // Get the version information:
    QDomElement elementRoot = domXML.documentElement();
    int iVersion = 0;
    QDomNode nodeVersion = TrackInfoObject::selectNode( elementRoot, "Version" );
    if (!nodeVersion.isNull() )
        iVersion = nodeVersion.toElement().text().toInt();

    // Get all the Playlists written in the xml file:
    QDomNode node = elementRoot.firstChild();

	while ( !node.isNull() )
    {
        if( node.isElement() && node.nodeName() == "Playlist")
		{
		QString sName = node.toElement().attribute( "Name" );
		qDebug("Testing: %s %s", currentPlaylist.latin1(), sName.latin1());
		if(sName == sPlaylist){
		int trackNo = 0;
		qDebug("Adding childs...");
		while(node.hasChildNodes())

		if ( node.firstChild().isElement() && node.firstChild().nodeName() == "Track" )
        {
            qDebug("Index is now: %d", trackNo);
			// Create a new track:
            TrackInfoObject *Track;
            Track = new TrackInfoObject(node.firstChild());
            Track->m_iIndex = trackNo;
			// Append it to the list of tracks:
            if (!fileExistsInList(Track->getFilename()))
            {
                // Shall we re-parse the header?:
                if (iVersion < TRACKLIST_VERSION)
                {
                    qDebug("Reparsed %s", Track->getFilename().latin1() );
                    if (Track->parse() == OK)
                        m_lTracks.append( Track );
                }
                else
                    m_lTracks.append( Track );


            }
        ++trackNo;
	    node.removeChild(node.firstChild());
		}
		currentPlaylist = sPlaylist;
		break;
	  }
	 }
	node = node.nextSibling();
	}

}
void TrackList::slotBrowseDir()
{
    const QString * playlistdir = new QString(wTree->m_sPlaylistdir);
	/**
	QFileDialog* fd = new QFileDialog(wTree->m_sPlaylistdir , QString::null , 0,QString::null , TRUE );
    //fd->setMode( QFileDialog::Directory );
    fd->setMode( QFileDialog::AnyFile );
	fd->setCaption("Please choose a collection name for this collection:");
    **/
	bool ok;
	QString text = QInputDialog::getText("Choose Collection name", "Please enter a Name for this collection:", QLineEdit::Normal,QString::null, &ok, m_pTableTracks,"textinput" );
    if ( ok && !text.isEmpty() ) {
        // user entered something and pressed OK

		currentPlaylist =  tr(text) + ".xml";
		writeXML();
	}

}
//Deletes all the tracks in the Tracktable
void TrackList::slotClearPlaylist()
{
	 // Delete contents of tabletrack
    m_pTableTracks->setNumRows(0);
	while (m_lTracks.count() != 0)
    {
      m_lTracks.removeFirst(); //Delete All old Tracks
    }
}
void TrackList::slotTreeClick(QListViewItem * item, const QPoint & pos, int a)
{
	// Display popup menu ...
    //ToDo: ContentsX() seems to be making problems with wtreelist
        QPoint globalPos = wTree->mapToGlobal(pos);
        globalPos -= QPoint(wTree->contentsX(), wTree->contentsY());
        treeSelectMenu->popup(globalPos);



}
void TrackList::slotDeletePlaylist()
{
	qDebug("Deleting Playlist: %s",wTree->selectedItem()->text(0).latin1());
	if(wTree->selectedItem()->text(0) != "Playlists" && wTree->selectedItem()->text(0) != "Archive" && wTree->selectedItem()->text(2).endsWith(".xml")){


	// Initialize xml file:
    QFile opmlFile(wTree->m_sPlaylistdir);
	QDomDocument domXML("Mixxx_Track_List");
    if (!domXML.setContent( &opmlFile))
    {
        QMessageBox::critical( 0,
                tr( "Critical Error" ),
                tr( "Parsing error for file %1" ).arg( wTree->m_sPlaylistdir.latin1() ) );
        opmlFile.close();
        // Try writing a new file:
        //WriteXML();
    }
    opmlFile.close();

    // Get the version information:
    QDomElement elementRoot = domXML.documentElement();
    int iVersion = 0;
    QDomNode nodeVersion = TrackInfoObject::selectNode( elementRoot, "Version" );
    if (!nodeVersion.isNull() )
        iVersion = nodeVersion.toElement().text().toInt();

    // Get all the Playlists written in the xml file:
    QDomNode node = elementRoot.firstChild();

	while ( !node.isNull() )
    {
		 if( node.isElement() && node.nodeName() == "Playlist"){
		 elementRoot.removeChild(node);
	     break;
		 }
		 node = node.nextSibling();
	}

	if (!opmlFile.open(IO_WriteOnly))
    {
        QMessageBox::critical(0,
                tr("Error"),
                tr("Cannot open file %1").arg(wTree->m_sPlaylistdir.latin1()));
        return;
    }

    // Write to the file:
    QTextStream Xml(&opmlFile);
    Xml.setEncoding(QTextStream::Unicode);
    Xml << domXML.toString();
    opmlFile.close();

    refreshPlaylist();

    }
}

void TrackList::slotUpdateTracklistFake(QString sDir)
{
    slotUpdateTracklist(sDir);
}

//Looks through the playlists and gets the first occurence of findText in filename
void TrackList::slotFindTrack()
{
    bool ok;
    QString findText = QInputDialog::getText("Tracksearch", "Please enter a trackname to lookup in your collection:", QLineEdit::Normal,QString::null, &ok, m_pTableTracks,"textinput" );
    if ( ok && !findText.isEmpty() )
    {
        // user entered something and pressed OK

        // Initialize xml file:
       QFile opmlFile(wTree->m_sPlaylistdir);
        QDomDocument domXML("Mixxx_Track_List");
        if (!domXML.setContent( &opmlFile))
        {
            QMessageBox::critical( 0,
                    tr( "Critical Error" ),
                    tr( "Parsing error for file %1" ).arg( wTree->m_sPlaylistdir.latin1() ) );
            opmlFile.close();
            // Try writing a new file:
            //writeXML();
        }
        opmlFile.close();

        // Get the version information:
        QDomElement elementRoot = domXML.documentElement();
        int iVersion = 0;
        QDomNode nodeVersion = TrackInfoObject::selectNode( elementRoot, "Version" );
        if (!nodeVersion.isNull() )
            iVersion = nodeVersion.toElement().text().toInt();

        // Get all the Playlists written in the xml file:
        QDomNode node = elementRoot.firstChild();
        //First search through the playlist then do focus stuff depending on location of track
        while ( !node.isNull() )
        {

            QString tempPlaylistName = node.toElement().attribute("Name");
            while(node.hasChildNodes())
            {
                qDebug(WWidget::selectNodeQString(node.firstChild(), "Title"));
                if ( node.firstChild().isElement() && node.firstChild().nodeName() == "Track" && WWidget::selectNodeQString(node.firstChild(), "Title").contains(findText,FALSE))
                {
                    if(tempPlaylistName == currentPlaylist)
                    {
                        for(int i=0;i<m_pTableTracks->numRows();i++)
                        {
                            if(m_pTableTracks->text(i,1).contains(findText,FALSE))
                            {
                                m_pTableTracks->ensureCellVisible (i,0);
                                m_pTableTracks->selectCells(i,1,i,1);
                                break;
                            }//fi
                        }//rof
                    }
                    else
                    {
                        switch( QMessageBox::warning( m_pTableTracks, "Track Found",
                                "The Track you searched for was found in playlist:"+tempPlaylistName+"\n"
                                "Should I load this playlist now(Your current playlist will be saved)?\n\n",
                                "Load Playlist and save current",
                                "Cancel", 0, 0, 1 ) )
                        {
                        case 0: // The user clicked the Load Playlist button or pressed Enter
                            //load playlist
                            writeXML();
                            slotUpdateTracklist(tempPlaylistName);
                            break;
                        case 1: // The user clicked the cancel button or pressed Escape
                            // exit
                            break;
                        }//hctiws

                    }//fi

                }//fi
                node.removeChild(node.firstChild());
            }//elihw
            node = node.nextSibling();
        }//elihw
    }//fi
    wTree->mousePressed=false;
}

void TrackList::slotUpdateTracklist( QString sDir )
{
//    qDebug("dir: %s",sDir.latin1());
    //if(m_lTracks.count() != 0)
    qDebug("Updating Tracklist: %s", sDir.latin1());


    // Delete contents of tabletrack
    m_pTableTracks->setNumRows(0);

    // Set the new directory:
    m_sDirectory = sDir;
    if(sDir.endsWith(".xml"))
    {
        while (m_lTracks.count() != 0)
        {
            m_lTracks.removeFirst(); //Delete All old Tracks
        }
        loadPlaylist( sDir );
    }
    // Make the newlist:
    updateTracklist();
}


