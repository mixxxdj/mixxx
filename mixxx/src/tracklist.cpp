/***************************************************************************
                          tracklist.cpp  -  description
                             -------------------
    begin                : 10 02 2003
    copyright            : (C) 2003 by Ingo Kossyk & Tue Haste Andersen
    email                : kossyki@cs.tu-berlin.de & haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include <qstring.h>
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
#ifdef __WIN__
  #include "soundsourcesndfile.h"
#endif
#ifdef __UNIX__
  #include "soundsourceaudiofile.h"
#endif
#include "soundsourcemp3.h"
#include "soundsourceoggvorbis.h"

#include "enginebuffer.h"
#include "reader.h"
#include "wtracktable.h"
#include "wtracktableitem.h"
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
    m_pBuffer1 = buffer1;
    m_pBuffer2 = buffer2;
    m_pNumberPos1 = pNumberPos1;
    m_pNumberPos2 = pNumberPos2;
    
	m_iCurTrackIdxCh1 = -1;
    m_iCurTrackIdxCh2 = -1;                  
	wTree = pTree;
    wTree->m_sPlaylistdir = sPlaylistdir;
	wTree->setRoot(m_sDirectory);
	wTree->setPlaylist(wTree->m_sPlaylistdir);
	
	//m_lTracks.setAutoDelete( TRUE ); // the list owns the objects
    
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
	playSelectMenu->insertItem("Save Playlist", this, SLOT(WriteXML()));
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
	WriteXML();
	
	
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
        //WriteXML();
    }
    opmlFile.close();
	
    // Get the version information:
    QDomElement elementRoot = domXML.documentElement();
  
	
    // Get all the Playlists written in the xml file:
    QDomNode node = elementRoot.firstChild();
    // Only write out the xml file if the current playlist is still contained in it(wasnt deleted beforehand but is still flagged aktive):
	while ( !node.isNull() )
    {
		 if( node.isElement() && node.nodeName() == "Playlist" && node.toElement().attribute( "Name" ) == currentPlaylist){
		 WriteXML();
	     break;
		 }
		 node = node.nextSibling();
	}
    
    
	
    // Delete all the tracks:
    for (unsigned int i=0; i<m_lTracks.count(); i++)
        delete m_lTracks.at(i);
}

void TrackList::UpdateTracklist(QDomDocument * domXML)
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
    if(!m_sDirectory.endsWith(".xml")){
	    bFilesAdded = AddFiles(m_sDirectory.latin1(),domXML,TRUE);
	    iRow= m_pTableTracks->numRows();
    }else{
		bFilesAdded = TRUE;
		}
	
	int iTrackno=0;
	int iTrackCount=m_lTracks.count();
	
	if(!bFilesAdded && m_lTracks.count() == m_pTableTracks->numRows()){
		
		iRow = 0;
		iTrackCount = 0;
	}		
	m_pTableTracks->insertRows(m_pTableTracks->numRows(), iTrackCount );
	for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next() )
    {
        if (Track->exists())
        {
			m_pTableTracks->setItem(iRow, COL_TITLE, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->getTitle(), typeText));
    		m_pTableTracks->setItem(iRow, COL_ARTIST, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->getArtist(), typeText));
	    	m_pTableTracks->setItem(iRow, COL_COMMENT, new WTrackTableItem(m_pTableTracks,QTableItem::WhenCurrent, Track->getComment(), typeText));
		    m_pTableTracks->setItem(iRow, COL_TYPE, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->getType(), typeText));
	    	m_pTableTracks->setItem(iRow, COL_DURATION, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->getDurationStr(), typeDuration));
		    m_pTableTracks->setItem(iRow, COL_BITRATE, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->getBitrateStr(), typeNumber));
    		m_pTableTracks->setItem(iRow, COL_INDEX, new WTrackTableItem(m_pTableTracks,QTableItem::Never, QString("%1").arg(Track->m_iIndex), typeText));
	       	iRow ++;
	    }
        iTrackno ++;
    }
    
	qDebug("Number of rows written: %d",iRow);

	// Find the track which has been played the most times:
	m_iMaxTimesPlayed = 1;
	for (unsigned int i=0; i<m_lTracks.count(); i++)
		if ( m_lTracks.at(i)->getTimesPlayed() > m_iMaxTimesPlayed)
			m_iMaxTimesPlayed = m_lTracks.at(i)->getTimesPlayed();

	// Update the scores for all the tracks:
	UpdateScores();

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
void TrackList::slotEndOfTrackCh1(double)
{
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
void TrackList::UpdateScores()
{
	/**for (unsigned int iRow=0; iRow<m_lTracks.count(); iRow++)
	{
		TrackInfoObject *track = m_lTracks.at( m_pTableTracks->text( iRow, COL_INDEX ).toInt() );
        m_pTableTracks->setItem(iRow, COL_SCORE, new WTrackTableItem(m_pTableTracks,QTableItem::Never,
                                QString("%1").arg( (int) ( 99*track->getTimesPlayed()/m_iMaxTimesPlayed ), 2 ), typeNumber ));
	}**/
}

/*
	Write the xml tree to the file:
*/
void TrackList::WriteXML()
{
	QPtrList<TrackInfoObject> tempTracks = m_lTracks;
	
 	bool newPlaylist = true;
	bool newFile = false;
	bool filesAdded = false;
	QFile opmlFile(wTree->m_sPlaylistdir);
	QDomDocument domXML("Mixxx_Track_List");
	//qDebug("PlaylistID is: %s", currentPlaylist.latin1());
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
    
	/** First transfer information from the comment field from the table to the Track:
    for (unsigned int iRow=0; iRow<m_pTableTracks->numRows(); iRow++)
    {
        if (m_pTableTracks->item(iRow, COL_INDEX))
        {
            m_lTracks.at( m_pTableTracks->item(iRow, COL_INDEX)->text().toUInt() )->m_sComment =
                m_pTableTracks->item(iRow, COL_COMMENT)->text();
        }
    }**/
   	
	// Ensure UTF16 encoding
	if(newFile){
	domXML.appendChild(domXML.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-16\""));
    
    // Set the document type
    
	QDomElement elementRoot = domXML.createElement( "Mixxx_Track_List" );
    domXML.appendChild(elementRoot);
	
    // Add version information:
    TrackInfoObject::addElement( domXML, elementRoot, "Version", QString("%1").arg( TRACKLIST_VERSION ) );
	}	
	//Try to find new files and add them in m_lTracks 
	filesAdded = AddFiles(wTree->getRoot()->text(2).latin1(),&domXML,FALSE);
	QDomElement elementRoot = domXML.documentElement();
    // Get all the tracks written in the xml file:
    QDomNode node = elementRoot.firstChild();
	
	if(!filesAdded){
		qDebug("No new files have been added to the global collection");	
		}else{
		// Append all the tracks:
   		for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next())
    	{
        QDomElement elementNew = domXML.createElement("Track");
        // See if we should add information from the comment field:
        Track->writeToXML(domXML, elementNew);
		elementRoot.appendChild(elementNew);
    	}
			
	}
    
	m_lTracks = tempTracks;	//Get the playlist back.
	
	while(!node.isNull()){
	if( node.isElement() && node.nodeName() == "Playlist" && node.toElement().attribute( "Name" ) == currentPlaylist){
            newPlaylist = false;			
	//First delete all old childs
		while(node.hasChildNodes())
			node.removeChild(node.firstChild());
		
	// Insert all the tracks:
    for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next())
    {
        QDomElement elementNew = domXML.createElement("TrackPTR");
        // See if we should add information from the comment field:
        //Track->WriteToXML(domXML, elementNew);
        elementNew.setAttribute("PTR_ID",Track->m_iIndex);
		
		node.appendChild(elementNew);
    }
  	}
	node = node.nextSibling();
	}		
    
	if(newPlaylist){
	QDomElement playlistElement = domXML.createElement( "Playlist" );
	playlistElement.setAttribute("Name",currentPlaylist);
	for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next())
    {
        QDomElement elementNew = domXML.createElement("TrackPTR");
        elementNew.setAttribute("PTR_ID",Track->m_iIndex);
		// See if we should add information from the comment field:
        //Track->WriteToXML(domXML, elementNew);
        playlistElement.appendChild(elementNew);
    }
	elementRoot.appendChild(playlistElement);
	}		
	//Dont touch any other playlist items
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
	

/*
	Adds the files given in <path> to the list of files.
	Returns true if any new files were in fact added.
*/
int TrackList::getTrackCount( QDomDocument * docXML ){
	QDomElement elementRoot = docXML->documentElement();
    // Get all the tracks written in the xml file:
    QDomNode node = elementRoot.firstChild();
	int count = 0;
	while ( !node.isNull() )
    {
	 if ( node.firstChild().isElement() && node.firstChild().nodeName() == "Track" )
				 count++;
	 node = node.nextSibling();
	}
	return count;
}
bool TrackList::AddFiles(const char *path, QDomDocument * docXML, bool parseRoot)
{    
	QPtrList<TrackInfoObject> tempTracks = m_lTracks;
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
		
		iTrackNumber = getTrackCount(docXML);
			
		// Check if the file exists in the list:
            TrackInfoObject *Track;
			Track = FileExistsInList( m_fileInfo.fileName(),docXML ); 
	
			if (!Track)
            {
                Track = new TrackInfoObject( m_fileInfo.dir().absPath(), m_fileInfo.fileName() );
                Track->m_iIndex = iTrackNumber;
				
                // Append the track to the list of tracks:
                if (ParseHeader(Track) == OK)
                {
                    m_lTracks.append(Track);
                    qDebug( "Added new track: %s", Track->getFilename().latin1() );
                    bFoundFiles = true;
                } 
                else
                    qWarning("Could not parse %s", m_fileInfo.fileName().latin1());
            } 
            else
			{	
			if(parseRoot){
				m_lTracks = tempTracks;
				TrackInfoObject *Track2 = FileExistsInList( m_fileInfo.fileName(), NULL);
				if(!Track2)
				{
				Track2 = new TrackInfoObject( dir.absPath(), m_fileInfo.fileName() );
                Track2->m_iIndex = Track->m_iIndex;
				
                // Append the track to the list of tracks:
                if (ParseHeader(Track) == OK)
                {
                    m_lTracks.append(Track);
                    qDebug( "Found new track: %s", Track->getFilename().latin1() );
                    bFoundFiles = true;
                } 
                else
                    qWarning("Could not parse %s", m_fileInfo.fileName().latin1());
				}
            // If it exists in the list already, it might not have been found in the
            // first place because it has been moved:
            if (!Track->exists())
            {
                Track->setFilepath(m_fileInfo.dirPath());
               // Track->exists()= true;
                qDebug("Refound %s", Track->getFilename().latin1() );
            }
		 }
	    }
	}}
	else
    {
        dir.setFilter(QDir::Dirs);
        const QFileInfoList dir_list = *dir.entryInfoList();
        QFileInfoListIterator dir_it(dir_list);
        QFileInfo *d;
        dir_it += 2; // Traverse past "." and ".."
        while ((d=dir_it.current()))
        {
            //qDebug("Execing addfile: %s",d->filePath().latin1());
			if (AddFiles(d->filePath(),docXML,false))
                bFoundFiles = true;
            ++dir_it;
        }
	
        // ... and then all the files:
        dir.setFilter(QDir::Files);
        dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3 *.ogg *.Ogg *.OGG");
        const QFileInfoList *list = dir.entryInfoList();
        QFileInfoListIterator it(*list);        // create list iterator
        QFileInfo *fi;                          // pointer for traversing
		
		
		int iTrackNumber = 0;
		
		iTrackNumber = m_lTracks.count();
		
        while ((fi=it.current()))
        {
            
			// Check if the file exists in the list:
            TrackInfoObject *Track;
			Track = FileExistsInList( fi->fileName(),docXML ); 
			if (!Track)
            {
                Track = new TrackInfoObject( dir.absPath(), fi->fileName() );
                Track->m_iIndex = iTrackNumber;
				
                // Append the track to the list of tracks:
                if (ParseHeader(Track) == OK)
                {
                    m_lTracks.append(Track);
                    //qDebug( "Found new track while looking through dir nonuser: %s", Track->getFilename().latin1() );
                    bFoundFiles = true;
                } 
                else
                    qWarning("Could not parse while looking through dir nonuser: %s", fi->fileName().latin1());
            } 
            else
            //User implied exec of function
			if(parseRoot){
				
			    TrackInfoObject *Track2 = FileExistsInList( fi->fileName(), NULL);
				if(!Track2)
				{
				Track2 = new TrackInfoObject( dir.absPath(), fi->fileName() );
                Track2->m_iIndex = Track->m_iIndex;
				
                // Append the track to the list of tracks:
                if (ParseHeader(Track) == OK)
                {
                    m_lTracks.append(Track);
                    //qDebug( "Found new track while looking through dir user: %s", Track->getFilename().latin1() );
                    bFoundFiles = true;
                } 
                else
                    qWarning("Could not parse while looking through dir user: %s", fi->fileName().latin1());
				}
				
			
			// If it exists in the list already, it might not have been found in the
            // first place because it has been moved:
            if (!Track->exists())
            {
                Track->setFilepath(fi->dirPath());
                //Track->m_bExist= true;
                qDebug("Refound %s", Track->getFilename().latin1() );
            }
            }
            ++it;   // goto next list element
        	++iTrackNumber;
			}
    }
    //if(parseRoot)
		//m_lTracks = tempTracks;
	return bFoundFiles;
}

/*
    Fill in the information in the track, by using the static member ParseHeader
    in the different SoundSource objects.
*/
int TrackList::ParseHeader( TrackInfoObject *Track )
{
    // Add basic information:
    Track->parse(); 

    // Find the type:
    QString sType = Track->getFilename().section(".",-1).lower();

    // Parse it using the sound sources:
    int iResult = ERR;
    if (sType == "wav")
#ifdef __WIN__
        iResult = SoundSourceSndFile::ParseHeader(Track);
#endif
#ifdef __UNIX__
        iResult = SoundSourceAudioFile::ParseHeader(Track);
#endif
    else if (sType == "mp3")
        iResult = SoundSourceMp3::ParseHeader(Track);
    else if (sType == "ogg")
        iResult = SoundSourceOggVorbis::ParseHeader(Track);

    // Try to sort out obviously erroneous parsings:
    int iBitrate = Track->getBitrate();
    if ((iBitrate <= 0) || (iBitrate > 5000))
        Track->setBitrate(0);
                    
    return iResult;
}

/*
    Returns the TrackInfoObject which has the filename sFilename.
*/
TrackInfoObject *TrackList::FileExistsInList( const QString sFilename,QDomDocument * docXML)
{
    //qDebug("Executing FileExistsInList");
	TrackInfoObject *Track;
	if(docXML != NULL){
	// Get the root object
    QDomElement elementRoot = docXML->documentElement();
	QDomNode node = elementRoot.firstChild();
	while ( !node.isNull() )
    {
	if ( node.isElement() && node.nodeName() == "Track" && WWidget::selectNodeQString(node, "Filename") == sFilename)
        {	
		return new TrackInfoObject(node);	
		}
	node = node.nextSibling();
	}
	
	
	}else{
	
	TrackInfoObject *Track;
	Track = m_lTracks.first();
    while ((Track) && (Track->getFilename() != sFilename) )
        Track = m_lTracks.next();
    return Track;
    }
	return NULL;

}

/*
    These two slots basically just routes information further, but adds
    track information.
*/
void TrackList::slotChangePlay_1(int idx)
{
    if (idx==-1)
       idx =  m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX ).toInt();
	
	//(m_iCurTrackIdxCh1 = m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX ).toInt();
    
	for (TrackInfoObject *track = m_lTracks.first(); track; track = m_lTracks.next())
    {
    //qDebug("Working on : %s",track->getFilename().latin1());
    if (track && track->m_iIndex == idx)
    {
        // Update score:
        track->incTimesPlayed();
        if (track->getTimesPlayed() > m_iMaxTimesPlayed)
            m_iMaxTimesPlayed = track->getTimesPlayed();
        UpdateScores();

        // Request a new track from the reader:
        m_pBuffer1->getReader()->requestNewTrack( track );

        // Write info
        m_pText1->setText( track->getInfo() );
    	break;
	}
  }
}
//Refreshes the Tree of the Playlist Repository
void TrackList::refreshPlaylist(){
	
	wTree->setPlaylist(wTree->m_sPlaylistdir);
}
void TrackList::slotChangePlay_2(int idx)
{
    if (idx==-1)
      idx =  m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX ).toInt();  
	
	//m_iCurTrackIdxCh2 = m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX).toInt();
     for (TrackInfoObject *track = m_lTracks.first(); track; track = m_lTracks.next())
    {
    
        
    if (track && track->m_iIndex == idx)
    {
        // Update score:
        track->incTimesPlayed();
        if (track->getTimesPlayed() > m_iMaxTimesPlayed)
            m_iMaxTimesPlayed = track->getTimesPlayed();
        UpdateScores();

        // Request a new track from the reader:
        m_pBuffer2->getReader()->requestNewTrack( track );
    
        // Write info
        m_pText2->setText( track->getInfo() );
    	break;
	}
  }
}
//Deletes a Track from the Table 
void TrackList::slotDeleteTrack(int idx)
{
	TrackInfoObject *Track;
	if (idx==-1){
		 idx = m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX).toInt();
	}
	int count = 0;
	for(Track = m_lTracks.first(); Track; Track = m_lTracks.next()){
		if(Track->m_iIndex == idx)
			break;
		count++;
		
	}
	
	//qDebug("Removing index at: %d", idx);
	m_lTracks.remove(count);
	if (Track)
	{
		for(int i= 0; i<m_lTracks.count(); i++){//Iterate through the whole table from point idx on
			//int t_idx = m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX).toInt();
			//m_pTableTracks->setItem(i,0,m_pTableTracks->item(o,0));//Item gets replaced by Item below
			Track = m_lTracks.at(i);
			m_pTableTracks->setItem(i, COL_TITLE, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->getTitle(), typeText));
    		m_pTableTracks->setItem(i, COL_ARTIST, new WTrackTableItem( m_pTableTracks,QTableItem::Never, Track->getArtist(), typeText));
	    	m_pTableTracks->setItem(i, COL_COMMENT, new WTrackTableItem( m_pTableTracks,QTableItem::WhenCurrent, Track->getComment(), typeText));
		    m_pTableTracks->setItem(i, COL_TYPE, new WTrackTableItem( m_pTableTracks,QTableItem::Never, Track->getType(), typeText));
	    	m_pTableTracks->setItem(i, COL_DURATION, new WTrackTableItem( m_pTableTracks,QTableItem::Never, Track->getDurationStr(), typeDuration));
		    m_pTableTracks->setItem(i, COL_BITRATE, new WTrackTableItem( m_pTableTracks,QTableItem::Never, Track->getBitrateStr(), typeNumber));
    		m_pTableTracks->setItem(i, COL_INDEX, new WTrackTableItem( m_pTableTracks,QTableItem::Never, QString("%1").arg(Track->m_iIndex), typeText));
				
		    }
		}
		m_pTableTracks->setNumRows(m_lTracks.count());
		
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
/** Can be called to Update the Root Directory in the TreeList **/
void TrackList::slotUpdateRoot(QString sDir)
{
	
	wTree->setRoot(sDir);
	
	
	}

void TrackList::loadPlaylist( QString sPlaylist , QDomDocument * domXML) {
	
	
	currentPlaylist = sPlaylist;
	// Get the version information:
    QDomElement elementRoot = domXML->documentElement();
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
		//qDebug("Testing: %s %s", currentPlaylist.latin1(), sName.latin1());
		if(sName == sPlaylist){				
		int trackNo = 0;
		//qDebug("Adding childs...");
		while(node.hasChildNodes())
		if ( node.firstChild().isElement() && node.firstChild().nodeName() == "TrackPTR" )
        {
            //qDebug("Index is now: %d", node.firstChild().toElement().attribute( "PTR_ID" ).toInt());
			int index = node.firstChild().toElement().attribute( "PTR_ID" ).toInt();
			QDomNode nodeTemp = elementRoot.firstChild();
			while(!nodeTemp.isNull())
			{
			if(nodeTemp.isElement() && nodeTemp.nodeName() == "Track" && WWidget::selectNodeQString(nodeTemp, "Index").toInt() == index ){
			// Create a new track:
            TrackInfoObject *Track;
            Track = new TrackInfoObject(nodeTemp);
            Track->m_iIndex = index;
			// Append it to the list of tracks:
            // Shall we re-parse the header?:
                if (iVersion < TRACKLIST_VERSION)
                {
                    qDebug("Reparsed %s", Track->getFilename().latin1() );
                    if (ParseHeader( Track ) == OK) 
                        m_lTracks.append( Track );
                }
                else
                    m_lTracks.append( Track );
			}
			nodeTemp = nodeTemp.nextSibling();
		}
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
		WriteXML();
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
    /**int iVersion = 0;
    QDomNode nodeVersion = TrackInfoObject::SelectNode( elementRoot, "Version" );
    if (!nodeVersion.isNull() )
        iVersion = nodeVersion.toElement().text().toInt();
	**/
    // Get all the Playlists written in the xml file:
    QDomNode node = elementRoot.firstChild();
    
	while ( !node.isNull() )
    {
		 if( node.isElement() && node.nodeName() == "Playlist" && wTree->selectedItem()->text(2) == node.toElement().attribute( "Name" )){
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
	slotClearPlaylist();
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



	
	if ( ok && !findText.isEmpty() ) {
        // user entered something and pressed OK
	// Initialize xml file:
    bool m_bTrackFound=false;	
    //Check if the track the user is looking for is located in the currently active playlist or do nothing
	for(int i=0;i<m_pTableTracks->numRows();i++){
				//qDebug("Looking in Tracktable");
				if(m_pTableTracks->text(i,1).contains(findText,FALSE)){
					m_pTableTracks->clearSelection(TRUE);
					m_pTableTracks->ensureCellVisible (i,0);
				    m_pTableTracks->selectCells(i,1,i,1);
					m_bTrackFound = true;
					break;
					}//fi
				}//rof
	if(!m_bTrackFound){	
    QFile opmlFile(wTree->m_sPlaylistdir);
	int tempIdx=-1;
	QString tempFileName = "";
	QString tempFilePath = "";
	
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
    /**int iVersion = 0;
    QDomNode nodeVersion = TrackInfoObject::SelectNode( elementRoot, "Version" );
    if (!nodeVersion.isNull() )
        iVersion = nodeVersion.toElement().text().toInt();
	**/
    // Get all the Playlists written in the xml file:
    QDomNode node = elementRoot.firstChild();
    //First search through the playlist then do focus stuff depending on location of track
	
	while ( !node.isNull() )
    {
		
		//Check if the track was found in the global collection
		if( node.isElement() && node.nodeName() == "Track" &&  WWidget::selectNodeQString(node, "Title").contains(findText,FALSE))
		{
			//qDebug("Looking in global collection");
			tempIdx = WWidget::selectNodeQString(node, "Index").toInt();
		    tempFileName = WWidget::selectNodeQString(node, "Filename");	
			tempFilePath = WWidget::selectNodeQString(node, "Filepath");
			break;
		}  //Check if the track is located in the users playlists.
		node = node.nextSibling();
	}	
	
	node = elementRoot.firstChild();
	while ( !node.isNull() )
    {
	if( node.isElement() && node.nodeName() == "Playlist" && tempIdx != -1 && node.toElement().attribute("Name") != currentPlaylist)
		{
		    //qDebug("Looking in not current Playlist");
			while(node.hasChildNodes()){
				//The Track was found in a playlist that is not loaded
				if(node.firstChild().isElement() && node.firstChild().nodeName() == "TrackPTR" && node.firstChild().toElement().attribute("PTR_ID").toInt() == tempIdx)
				switch( QMessageBox::warning( m_pTableTracks, "Track Found",
                 "The Track you searched for was found in playlist:"+node.toElement().attribute("Name")+"\n"
            	 "Should I load this playlist now(Your current playlist will be saved)?\n\n",
                 "Load Playlist and save current",
                 "Cancel", 0, 0, 1 ) )	
                 {
          case 0: // The user clicked the Load Playlist button or pressed Enter
                  //load playlist
                  WriteXML();
		          slotUpdateTracklist(node.toElement().attribute("Name"));
		          wTree->mousePressed=false;
		  return;
          case 1: // The user clicked the cancel button or pressed Escape
                  // exit
          wTree->mousePressed=false;
		  return;
               }//hctiws   
			node.removeChild(node.firstChild());
		   }		  
		
		} 
		node = node.nextSibling();
	}
	
	//And finally if only the track was found in the global collection , give the user a choice
	if (tempIdx != -1)
	switch( QMessageBox::warning( m_pTableTracks, "Track Found in collection",
                 "The Track you searched for was found in the global collection\n"
            	 "Should I load this track now into your current playlist?\n\n",
                 "Add Track",
                 "Cancel", 0, 0, 1 ) )	
                 {
          case 0: // The user clicked the Add Track button or pressed Enter
                  //add track
                  //qDebug("Adding : %s", tempFileName.latin1());
		  		  
		          slotUpdateTracklist(tempFilePath+"/"+tempFileName);
		          wTree->mousePressed=false;
		  return;
          case 1: // The user clicked the cancel button or pressed Escape
                  // exit
                  wTree->mousePressed=false;
		  return;
               }//hctiws 
	
		   }
	   }  
  wTree->mousePressed=false;
 }

void TrackList::slotUpdateTracklist( QString sDir )
{
//    qDebug("dir: %s",sDir.latin1());
	//if(m_lTracks.count() != 0)
	qDebug("Updating Tracklist: %s", sDir.latin1());
    
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
    // Delete contents of tabletrack
    m_pTableTracks->setNumRows(0);

    // Set the new directory:
    m_sDirectory = sDir;
	if(sDir.endsWith(".xml")){
 	while (m_lTracks.count() != 0)
    {
      m_lTracks.removeFirst(); //Delete All old Tracks
    }	
		loadPlaylist( sDir , &domXML);
    
	}	
	// Make the newlist:
    UpdateTracklist(&domXML);
	
		}
