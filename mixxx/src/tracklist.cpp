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
    m_pTrack1 = 0;
    m_pTrack2 = 0;

    wTree = pTree;
    if (wTree)
    {
        wTree->m_sPlaylistdir = sPlaylistdir;
        wTree->setRoot(m_sDirectory);
        wTree->setPlaylist(wTree->m_sPlaylistdir);
    }

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
	playSelectMenu->insertItem("Save Playlist", this, SLOT(slotSavePls()));
	playSelectMenu->insertItem("Save Playlist as", this, SLOT(slotSavePlsAs()));
	playSelectMenu->insertItem("Delete Track",this, SLOT(slotDeleteTrack()));
	playSelectMenu->insertItem("Clear Playlist", this, SLOT(slotClearPlaylist()));
	playSelectMenu->insertSeparator(2);
	playSelectMenu->insertSeparator(5);
	// Connect the right click to the slot where the menu is shown:
    if (m_pTableTracks)
        connect(m_pTableTracks, SIGNAL(pressed(int, int, int, const QPoint &)),
                                SLOT(slotClick(int, int, int, const QPoint &)));
    if (wTree)
        connect(wTree, SIGNAL(rightButtonPressed ( QListViewItem *, const QPoint &, int )),
                       SLOT(slotTreeClick(QListViewItem *, const QPoint &, int)));

    //connect applyDir signal with the UpdateTracklist slot, as described in wTreeList
    if (m_pTableTracks)
        connect(m_pTableTracks, SIGNAL(applyDir(QString )), this, SLOT(slotUpdateTracklist( QString  )));
    if (wTree)
        connect(wTree, SIGNAL(loadPls(QString)), this, SLOT(slotUpdateTracklistFake( QString) ) );

    //WriteXML();
	qDebug("Loading Collection");
	loadCollection();

}

TrackList::~TrackList()
{
    qDebug("destroying tracklist");
	 /**
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
    **/

    WriteXML();
    unsigned int i;
    for (i=0; i<m_lPlaylist.count(); i++)
        delete m_lPlaylist.at(i);

    // Delete all the tracks:
    for (i=0; i<m_lTracks.count(); i++)
        delete m_lTracks.at(i);
}

//Loads the Main Collection into m_lTracks
void TrackList::loadCollection()
{
    if (!wTree)
        return;

    QFile opmlFile(wTree->m_sPlaylistdir);
	QDomDocument domXML("Mixxx_Track_List");
	//qDebug("PlaylistID is: %s", currentPlaylist.latin1());
	if (!opmlFile.exists()){

	qDebug("Could not open the default playlist file!");
	return;
		}
    if (!domXML.setContent( &opmlFile))
    {

        opmlFile.close();

        //WriteXML();
    }

	opmlFile.close();

	// Get the version information:
    QDomElement elementRoot = domXML.documentElement();

    // Get all the Playlists written in the xml file:
    QDomNode node = elementRoot.firstChild();

	while ( !node.isNull() )
    {
        if( node.isElement() && node.nodeName() == "Track")
		{
		 TrackInfoObject *Track = new TrackInfoObject(node);

		if(Track->exists()){
		   Track->m_iIndex = WWidget::selectNodeQString(node, "Index").toInt();
            m_lTracks.append(Track);
		   }
		}
	        node= node.nextSibling();
		}

}
//Loads a Playlist into the TrackTable
void TrackList::UpdateTracklistFromPls(){
	int iRow = 0;
	TrackInfoObject *Track;
        int i;
	for(i=0; i<m_lPlaylist.count();i++){
	   Track = m_lPlaylist.at(i);
		iRow = m_pTableTracks->numRows();
		m_pTableTracks->insertRows(iRow, 1);

		if (Track->exists())
        {
			Track->insertInTrackTableRow(m_pTableTracks , iRow , Track->m_iIndex);

	    }

	}
	// Find the track which has been played the most times:
	m_iMaxTimesPlayed = 1;
	for (i=0; i<m_lPlaylist.count(); i++)
		if ( m_lPlaylist.at(i)->getTimesPlayed() > m_iMaxTimesPlayed)
			m_iMaxTimesPlayed = m_lPlaylist.at(i)->getTimesPlayed();

}
void TrackList::UpdateTracklist(QDomDocument * domXML)
{
    // Run through all the files and add the new ones to the xml file:
	bool bFilesAdded = false;
	//Save the playlist
	
	// Put information from all the tracks into the table:
	/************************************************************
	the following code has been added for debugging
	tracks should only be added to the table list if there are new ones
	in the m_lTracks Pointer list. This is being determined by AddFiles
	anyhow (checks if files are existing in the tracklist allready, if so
	they have to be existing as an Item in the tablelist by now)
	************************************************************/
	
	int iRow=0;
    int iRowTracks=0;
	if(!m_sDirectory.endsWith(".xml")){
	    iRowTracks = m_lPlaylist.count();
		bFilesAdded = AddFiles(m_sDirectory.latin1(),domXML);
    	iRow = m_pTableTracks->numRows();
		qDebug("Number of rows in table : %d" , iRow);
    }else{
		bFilesAdded = TRUE;
		}
	
	
	
	
	TrackInfoObject *Track;
        int i;
	for(i=iRowTracks; i<m_lPlaylist.count();i++){
	Track = m_lPlaylist.at(i);	
		qDebug("Working on Track: %d",Track->m_iIndex);
	bool trackExistsinTable = false;
	
	if(iRow !=0)
	for(i = 0; i< iRow;i++){	 
		int idx = m_pTableTracks->text(i, COL_INDEX).toInt();
		if(Track->m_iIndex == idx){
		trackExistsinTable = true;
	     qDebug("Track allready exists");
		}
		}
    if(!trackExistsinTable){
	m_pTableTracks->insertRows(iRow, 1);	
	
		if (Track->exists())
        {
			Track->insertInTrackTableRow(m_pTableTracks , iRow, Track->m_iIndex);
	       	iRow ++;
	    }
		
	}
	}
    
	qDebug("Number of rows written: %d",iRow);

	// Find the track which has been played the most times:
	m_iMaxTimesPlayed = 1;
	for (i=0; i<m_lPlaylist.count(); i++)
		if ( m_lPlaylist.at(i)->getTimesPlayed() > m_iMaxTimesPlayed)
			m_iMaxTimesPlayed = m_lPlaylist.at(i)->getTimesPlayed();

	// Update the scores for all the tracks:
	UpdateScores();
		//WriteXML(&tempTracks);
    
}
void TrackList::loadTrack1(QString name)
{
    if (QFile(name).exists())
    {
        TrackInfoObject *t = new TrackInfoObject("",name);
        m_pBuffer1->getReader()->requestNewTrack(t);
        if (m_pText1)
            m_pText1->setText(t->getInfo());
    }
}
void TrackList::loadTrack2(QString name)
{
    if (QFile(name).exists())
    {
        TrackInfoObject *t = new TrackInfoObject("",name);
        m_pBuffer2->getReader()->requestNewTrack(t);
        if (m_pText2)
            m_pText2->setText(t->getInfo());
    }
}

TrackInfoObject *TrackList::getTrackInfo1()
{
    return m_pTrack1;
}

TrackInfoObject *TrackList::getTrackInfo2()
{
    return m_pTrack2;
}

void TrackList::slotEndOfTrackCh1(double)
{
    int iIndex = 0;
	switch ((int)m_pEndOfTrackModeCh1->getValue())
    {
    case TRACK_END_MODE_NEXT:
        // Load next track
        if(m_lPlaylist.count() != 0)
			for(TrackInfoObject * plsTrack = m_lPlaylist.first();plsTrack;plsTrack = m_lPlaylist.next())
            {
             if(plsTrack && plsTrack->m_iIndex == m_iCurTrackIdxCh1)
			 {
				 plsTrack = m_lPlaylist.next();
				 if(plsTrack)
				 iIndex =  plsTrack->m_iIndex;
			 }
            
            }				
	
	    if(iIndex != 0)
        slotChangePlay_1(iIndex);
        break;

    default:
        qDebug("Invalid EndOfTrack mode value");
    }
    m_pEndOfTrackCh1->setValueFromApp(0.);
}

void TrackList::slotEndOfTrackCh2(double)
{
    int iIndex = 0;
	switch ((int)m_pEndOfTrackModeCh2->getValue())
    {
    case TRACK_END_MODE_NEXT:
        // Load next track
        if(m_lPlaylist.count() != 0)
			for(TrackInfoObject * plsTrack = m_lPlaylist.first();plsTrack;plsTrack = m_lPlaylist.next())
            {
             if(plsTrack && plsTrack->m_iIndex == m_iCurTrackIdxCh2)
			 {
				 plsTrack = m_lPlaylist.next();
				 if(plsTrack)
				 iIndex =  plsTrack->m_iIndex;
			 }
            
            }				
	
	    if(iIndex != 0)
        slotChangePlay_2(iIndex);
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
	if(m_lPlaylist.count() !=0)
		for(unsigned int iRow=0; iRow<m_lPlaylist.count();iRow++)
		{
			TrackInfoObject *track = m_lPlaylist.at( iRow );
			track->setScore( (int) ( 99*track->getTimesPlayed()/m_iMaxTimesPlayed ));
			
		}
	
	
	/*if(m_lPlaylist.count() != 0 && m_lTracks.count() != 0)
	for (unsigned int iRow=0; iRow<m_lPlaylist.count(); iRow++)
	{
		TrackInfoObject *track = m_lPlaylist.at( iRow );
		
		for(TrackInfoObject *track2 = m_lTracks.first();track2;track2 = m_lTracks.next())
		{
			if(track->m_iIndex == track2->m_iIndex){
				m_pTableTracks->setItem(iRow, COL_SCORE, new WTrackTableItem(m_pTableTracks,QTableItem::Never,
                                QString("%1").arg( (int) ( 99*track->getTimesPlayed()/m_iMaxTimesPlayed ), 2 ), typeNumber ));
	            //track->setScore((int) ( 99*track->getTimesPlayed()/m_iMaxTimesPlayed ));
				//track2->setScore((int) ( 99*track->getTimesPlayed()/m_iMaxTimesPlayed ));
				}
		}
        
	}*/
}

/*
	Write the xml tree to the file:
*/
void TrackList::WriteXML()
{
	bool newFile = false;

    if (!wTree)
        return;

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
	
	//qDebug("Writing ID %s, %d tracks", currentPlaylist.latin1(),m_pTableTracks->numRows());
   	
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
	//filesAdded = AddFiles(wTree->getRoot()->text(2).latin1(),&domXML,FALSE);
	QDomElement elementRoot = domXML.documentElement();
    // Get all the tracks written in the xml file:
    QDomNode node = elementRoot.firstChild();
	
	int iRow = m_pTableTracks->numRows();
	for (TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next())
    {	
	bool fileFoundInXML = false;
	
	while(!node.isNull()){
	// Insert all the tracks:
    if(node.isElement() && node.nodeName() == "Track" && WWidget::selectNodeQString(node, "Index").toInt() == Track->m_iIndex ){
		fileFoundInXML=true;
        break;
	}	 
 	node = node.nextSibling();
	}
	if(!fileFoundInXML){
			QDomElement elementTemp = domXML.createElement("Track");
            Track->writeToXML(domXML,elementTemp);
            elementRoot.appendChild(elementTemp);			
	     }		 
    }
    
	
	
    //Dont touch any other playlist items
	if (!opmlFile.open(IO_WriteOnly))
    {
        qWarning(tr("Error: Cannot open file %1").arg(wTree->m_sPlaylistdir.latin1()));
		/**QMessageBox::critical(0,
                tr("Error"),
                tr("Cannot open file %1").arg(wTree->m_sPlaylistdir.latin1()));**/
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
	Iterates through the global collection and returns index which is highest +1
*/
int TrackList::getNewTrackIndex(){
	int iIndex = 0;
	
	for(TrackInfoObject *Track = m_lTracks.first(); Track; Track = m_lTracks.next()){
		if(Track->m_iIndex > iIndex)
            iIndex= Track->m_iIndex;			
		}
		
		return iIndex+1;
	}
bool TrackList::AddFiles(const char *path, QDomDocument * docXML)
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
			
		
		
		    
			int iTrackNumber = getNewTrackIndex();
		    qDebug("Tracknumber is: %d" + iTrackNumber);			
			// Check if the file exists in the list:
            TrackInfoObject *Track;
			Track = FileExistsInList( m_fileInfo.fileName(),docXML, -1 ); 
	
			if (!Track)
            {
                //qDebug("Single Track not found in list: %s", m_fileInfo.fileName().latin1());
				
				Track = new TrackInfoObject( m_fileInfo.dir().absPath(), m_fileInfo.fileName() );
                Track->m_iIndex = iTrackNumber;
				
                // Append the track to the list of tracks:
                if (ParseHeader(Track) == OK)
                {
                    m_lPlaylist.append(Track);
					m_lTracks.append(Track);
                    qDebug( "Added new track: %s", Track->getFilename().latin1() );
                    bFoundFiles = true;
                } 
                else
                    qWarning("Could not parse %s", m_fileInfo.fileName().latin1());
            if (!Track->exists())
            {
                Track->setFilepath(m_fileInfo.dirPath());
               // Track->exists()= true;
                qDebug("Refound %s", Track->getFilename().latin1() );
            }
			}else {
             if (ParseHeader(Track) == OK){
				 bool bFoundTrack = false;
				 for(TrackInfoObject *trackpls = m_lPlaylist.first();trackpls;trackpls=m_lPlaylist.next())
				 		if(trackpls->m_iIndex == Track->m_iIndex)
							bFoundTrack = true;
						
						if(!bFoundTrack)
							m_lPlaylist.append(Track);
						
			     bFoundFiles = true;
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
			if (AddFiles(d->filePath(),docXML)){
                bFoundFiles = true;
            	WriteXML();
				}
			++dir_it;
        }
	
        // ... and then all the files:
        dir.setFilter(QDir::Files);
        dir.setNameFilter("*.wav *.Wav *.WAV *.mp3 *.Mp3 *.MP3 *.ogg *.Ogg *.OGG");
        const QFileInfoList *list = dir.entryInfoList();
        QFileInfoListIterator it(*list);        // create list iterator
        QFileInfo *fi;                          // pointer for traversing
		
		
		
		
		
		int iTrackNumber = getNewTrackIndex();
        
		while ((fi=it.current()))
        {
           
			// Check if the file exists in the list:
            TrackInfoObject *Track;
			Track = FileExistsInList( fi->fileName(),docXML, iTrackNumber ); 
			if (!Track)
            {
                qDebug("Found new Track while parsing dir.Filename: %s", fi->fileName().latin1());
				
				Track = new TrackInfoObject( dir.absPath(), fi->fileName() );
                Track->m_iIndex = iTrackNumber;
				
                // Append the track to the list of tracks:
                if (ParseHeader(Track) == OK)
                {
                    m_lPlaylist.append(Track);
					m_lTracks.append(Track);
                    qDebug( "Tracknumber %d", iTrackNumber );
                    ++iTrackNumber;
					bFoundFiles = true;
                } 
                else
                    qWarning("Could not parse while looking through dir nonuser: %s", fi->fileName().latin1());
                    /** If it exists in the list already, it might not have been found in the
                    // first place because it has been moved:
                    if (!Track->exists())
                    {
                     Track->setFilepath(fi->dirPath());
                     //Track->m_bExist= true;
                     qDebug("Refound %s", Track->getFilename().latin1() );
                     }  **/
				}else{
                 qDebug("Found Track with index: %d",Track->m_iIndex);
				 if (ParseHeader(Track) == OK){
				 bool bFoundTrack = false;
					 for(TrackInfoObject *trackpls = m_lPlaylist.first();trackpls;trackpls=m_lPlaylist.next())
				 		if(trackpls->m_iIndex == Track->m_iIndex)
							bFoundTrack = true;
						
						if(!bFoundTrack)
							m_lPlaylist.append(Track);			     
						
						bFoundFiles = true;
				 }

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
TrackInfoObject *TrackList::FileExistsInList( const QString sFilename,QDomDocument * docXML, int Index)
{
    //qDebug("Executing FileExistsInList");
	
	if(Index == -1){
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
    if(m_lPlaylist.count() != 0)
	for (TrackInfoObject *track = m_lTracks.first(); track; track = m_lTracks.next())
    {
    //qDebug("Working on : %s",track->getFilename().latin1());
    if (track && track->m_iIndex == idx)
    {
        m_pTrack1 = track;
        emit(signalNewTrack1(track));

        // Update score:
        track->incTimesPlayed();
        if (track->getTimesPlayed() > m_iMaxTimesPlayed)
            m_iMaxTimesPlayed = track->getTimesPlayed();
        UpdateScores();
        
		//Set the current Track index so that slotEndOfTrack knows which track to play next
		m_iCurTrackIdxCh1 = track->m_iIndex;
		
        // Request a new track from the reader:
        m_pBuffer1->getReader()->requestNewTrack( track );

        // Write info
        if (m_pText1)
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
    if(m_lPlaylist.count() != 0)
	for (TrackInfoObject *track = m_lTracks.first(); track; track = m_lTracks.next())
    {


    if (track && track->m_iIndex == idx)
    {
        m_pTrack2 = track;
        emit(signalNewTrack2(track));

        //Set the current Track index so that slotEndOfTrack knows which track to play next
		m_iCurTrackIdxCh2 = track->m_iIndex;

		// Update score:
        track->incTimesPlayed();
        if (track->getTimesPlayed() > m_iMaxTimesPlayed)
            m_iMaxTimesPlayed = track->getTimesPlayed();
        UpdateScores();
        m_iCurTrackIdxCh2 = idx;
        // Request a new track from the reader:
        m_pBuffer2->getReader()->requestNewTrack( track );

        // Write info
        if (m_pText2)
            m_pText2->setText( track->getInfo() );
    	return;
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
	for(Track = m_lPlaylist.first(); Track; Track = m_lPlaylist.next()){
		if(Track->m_iIndex == idx)
		{	
		Track->removeFromTrackTable();
		break;
		}
		count++;
		
	}
	m_lPlaylist.remove(count);
	refreshTrackTableContents(1);
	
	qDebug("Removed playlist item at: %d", count);
	
	/**
	if (Track)
	{
		for(int i= 0; i<m_lPlaylist.count(); i++){//Iterate through the whole table from point idx on
			//int t_idx = m_pTableTracks->text(m_pTableTracks->currentRow(), COL_INDEX).toInt();
			//m_pTableTracks->setItem(i,0,m_pTableTracks->item(o,0));//Item gets replaced by Item below
			Track = m_lPlaylist.at(i);
			m_pTableTracks->setItem(i, COL_TITLE, new WTrackTableItem(m_pTableTracks,QTableItem::Never, Track->getTitle(), typeText));
    		m_pTableTracks->setItem(i, COL_ARTIST, new WTrackTableItem( m_pTableTracks,QTableItem::Never, Track->getArtist(), typeText));
	    	m_pTableTracks->setItem(i, COL_COMMENT, new WTrackTableItem( m_pTableTracks,QTableItem::WhenCurrent, Track->getComment(), typeText));
		    m_pTableTracks->setItem(i, COL_TYPE, new WTrackTableItem( m_pTableTracks,QTableItem::Never, Track->getType(), typeText));
	    	m_pTableTracks->setItem(i, COL_DURATION, new WTrackTableItem( m_pTableTracks,QTableItem::Never, Track->getDurationStr(), typeDuration));
		    m_pTableTracks->setItem(i, COL_BITRATE, new WTrackTableItem( m_pTableTracks,QTableItem::Never, Track->getBitrateStr(), typeNumber));
    		m_pTableTracks->setItem(i, COL_INDEX, new WTrackTableItem( m_pTableTracks,QTableItem::Never, QString("%1").arg(Track->m_iIndex), typeText));
				
		    }
		}
		m_pTableTracks->setNumRows(m_lPlaylist.count());
		**/
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
            qDebug("ptrPlaylist contains: %d", m_lPlaylist.count());
			qDebug("Index is now: %d", node.firstChild().toElement().attribute( "PTR_ID" ).toInt());
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
                   if (ParseHeader( Track ) == OK){ 
                        m_lPlaylist.append( Track );
				        bool foundTrack = false;
					for(TrackInfoObject *tempTrack = m_lTracks.first();tempTrack;tempTrack = m_lTracks.next())
						if(tempTrack->m_iIndex == Track->m_iIndex)
							foundTrack = true;
					
						if(!foundTrack)
							m_lTracks.append(Track);
					}
				}
                else
                    m_lPlaylist.append( Track );
				bool foundTrack = false;
					for(TrackInfoObject *tempTrack = m_lTracks.first();tempTrack;tempTrack = m_lTracks.next())
						if(tempTrack->m_iIndex == Track->m_iIndex)
							foundTrack = true;
					
						if(!foundTrack)
							m_lTracks.append(Track);
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
void TrackList::slotSavePlsAs()
{
  
	//Update any not saved Tracks into the main collection
	WriteXML();
	QFile opmlFile(wTree->m_sPlaylistdir);
	QDomDocument domXML("Mixxx_Track_List");
	//qDebug("PlaylistID is: %s", currentPlaylist.latin1());
	if (!opmlFile.exists()){
        
	qDebug("Could not open the default playlist file!");
	
		}
    if (!domXML.setContent( &opmlFile))
    {
       
        opmlFile.close();
        
        //WriteXML();
    }
    
	opmlFile.close();
	bool ok;
	
	QString text = QInputDialog::getText("Choose Collection name", "Please enter a Name for this collection:", QLineEdit::Normal,QString::null, &ok, m_pTableTracks,"textinput" );
    if ( ok && !text.isEmpty() ) {
        // user entered something and pressed OK
    	
		currentPlaylist =  tr(text) + ".xml";
		
	}
	
	QDomElement elementRoot = domXML.documentElement();

	QDomElement playlistElement = domXML.createElement( "Playlist" );
	playlistElement.setAttribute("Name",currentPlaylist);
	for (TrackInfoObject *Track = m_lPlaylist.first(); Track; Track = m_lPlaylist.next())
    {
        QDomElement elementNew = domXML.createElement("TrackPTR");
        elementNew.setAttribute("PTR_ID",Track->m_iIndex);
		// See if we should add information from the comment field:
        //Track->WriteToXML(domXML, elementNew);
         playlistElement.appendChild(elementNew);
    }
	elementRoot.appendChild(playlistElement);
		
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
void TrackList::slotSavePls()
{

	//Update any not saved Tracks into the main collection
	WriteXML();
	QFile opmlFile(wTree->m_sPlaylistdir);
	QDomDocument domXML("Mixxx_Track_List");
	//qDebug("PlaylistID is: %s", currentPlaylist.latin1());
	if (!opmlFile.exists()){
        
	qDebug("Could not open the default playlist file!");
	
		}
    if (!domXML.setContent( &opmlFile))
    {
       
        opmlFile.close();
        
        //WriteXML();
    }
    
	opmlFile.close();
	
	QDomElement elementRoot = domXML.documentElement();
    // Get all the tracks written in the xml file:
    QDomNode node = elementRoot.firstChild();
	while(!node.isNull()){
	
		
	if( node.isElement() && node.nodeName() == "Playlist" && node.toElement().attribute( "Name" ) == currentPlaylist){
       		
	//First delete all old childs
		while(node.hasChildNodes())
			node.removeChild(node.firstChild());
		
	for(TrackInfoObject *Track = m_lPlaylist.first();Track;Track = m_lPlaylist.next()){
		QDomElement elementNew = domXML.createElement("TrackPTR");
        // See if we should add information from the comment field:
        //Track->WriteToXML(domXML, elementNew);
        elementNew.setAttribute("PTR_ID",Track->m_iIndex);
		node.appendChild(elementNew);
    }
		}
  	node = node.nextSibling();
	}
	
		
	//Dont touch any other playlist items
	if (!opmlFile.open(IO_WriteOnly))
    {
        qWarning(tr("Error: Cannot open file %1").arg(wTree->m_sPlaylistdir.latin1()));
		/**QMessageBox::critical(0,
                tr("Error"),
                tr("Cannot open file %1").arg(wTree->m_sPlaylistdir.latin1()));**/
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
//Deletes all the tracks in the Tracktable
void TrackList::slotClearPlaylist()
{
	for(TrackInfoObject *Track = m_lPlaylist.first();Track;Track = m_lPlaylist.next())
    {
	  Track->removeFromTrackTable();	
	}
	while(m_lPlaylist.count() != 0)
		m_lPlaylist.removeFirst(); //Delete All old Tracks
	
	// Delete contents of tabletrack
    m_pTableTracks->setNumRows(0);	
	m_pTableTracks->clearSelection(TRUE);
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
    //Delete all items in current playlist list , if the current playlist matched the one being deleted.
	if(tr(wTree->selectedItem()->text(2)) == currentPlaylist)
	slotClearPlaylist();
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
                  //WriteXML();
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
/** Sorts the Tracktable ascending index and substracts iCount of numRows**/
void TrackList::refreshTrackTableContents(int iCount){
	
	m_pTableTracks->sortColumn(COL_INDEX,TRUE,TRUE);
	
	m_pTableTracks->setNumRows(m_pTableTracks->numRows() - iCount);
	
	
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
  
	
    // Set the new directory:
    m_sDirectory = sDir;
	if(sDir.endsWith(".xml")){
 	/**while (m_lTracks.count() != 0)
    {
      m_lTracks.removeFirst(); //Delete All old Tracks
    }	**/	
	while (m_lPlaylist.count() != 0)
	{
		m_lPlaylist.removeFirst();
	}
	// Delete contents of tabletrack
    m_pTableTracks->setNumRows(0);
	loadPlaylist( sDir , &domXML);
    UpdateTracklistFromPls();
	return;
	}else
	// Make the newlist:
    UpdateTracklist(&domXML);
	
		}
