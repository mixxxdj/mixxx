//
// C++ Implementation: track
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "track.h"
#include "trackinfoobject.h"
#include "trackcollection.h"
#include "xmlparse.h"
#include <qfile.h>
#include "mixxxview.h"
#include <qdragobject.h>
#include "wtracktable.h"
#include "wtreeview.h"
#include "wnumberpos.h"
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qcstring.h>
#include "enginebuffer.h"
#include "reader.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "configobject.h"
#include "trackimporter.h"
#include "wavesummary.h"
#include "woverview.h"

Track::Track(QString location, MixxxView *pView, EngineBuffer *pBuffer1, EngineBuffer *pBuffer2, WaveSummary *pWaveSummary)
{
    m_pView = pView;
    m_pBuffer1 = pBuffer1;
    m_pBuffer2 = pBuffer2;
    m_pActivePlaylist = 0;
    m_pActivePopupPlaylist = 0;
    m_pTrackPlayer1 = 0;
    m_pTrackPlayer2 = 0;
    m_pWaveSummary = pWaveSummary;

    m_pTrackCollection = new TrackCollection();
    m_pTrackImporter = new TrackImporter(m_pView,m_pTrackCollection);

    // Read the XML file
    readXML(location);

    // Ensure that one playlist is present
    if (m_qPlaylists.count()==0)
        m_qPlaylists.append(new TrackPlaylist(m_pTrackCollection));

    // Update tree view
    updatePlaylistViews();

    // Insert the first playlist in the list
    m_pActivePlaylist = m_qPlaylists.at(0);
    m_pActivePlaylist->activate(m_pView->m_pTrackTable);

    // Connect mouse events from the tree view
    connect(m_pView->m_pTreeView, SIGNAL(activatePlaylist(QString)), this, SLOT(slotActivatePlaylist(QString)));
    connect(this, SIGNAL(activePlaylist(TrackPlaylist *)), m_pView->m_pTreeView, SLOT(slotHighlightPlaylist(TrackPlaylist *)));
    // Connect drop events to table
    connect(m_pView->m_pTrackTable, SIGNAL(dropped(QDropEvent *)), this, SLOT(slotDrop(QDropEvent *)));

    // Connect mouse events from WTrackTable
    connect(m_pView->m_pTrackTable, SIGNAL(mousePressed(TrackInfoObject*, int )), this, SLOT(slotTrackPopup(TrackInfoObject*, int )));

    // Get ControlObject for determining end of track mode, and set default value to STOP.
    m_pEndOfTrackModeCh1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","TrackEndMode")));
    m_pEndOfTrackModeCh2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]","TrackEndMode")));

    // Connect end-of-track signals to this object
    m_pEndOfTrackCh1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","TrackEnd")));
    m_pEndOfTrackCh2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]","TrackEnd")));
    connect(m_pEndOfTrackCh1, SIGNAL(valueChanged(double)), this, SLOT(slotEndOfTrackPlayer1(double)));
    connect(m_pEndOfTrackCh2, SIGNAL(valueChanged(double)), this, SLOT(slotEndOfTrackPlayer2(double)));

    // Get play buttons
    m_pPlayButtonCh1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","play")));
    m_pPlayButtonCh2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]","play")));

    // Play position for each player. Used to determine which track to load next
    m_pPlayPositionCh1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","playposition")));
    m_pPlayPositionCh2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]","playposition")));
    
    // Make controls for next and previous track
    m_pNextTrackCh1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]","NextTrack")));
    m_pPrevTrackCh1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]","PrevTrack")));
    m_pNextTrackCh2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]","NextTrack")));
    m_pPrevTrackCh2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]","PrevTrack")));
    connect(m_pNextTrackCh1, SIGNAL(valueChanged(double)), this, SLOT(slotNextTrackPlayer1(double)));
    connect(m_pPrevTrackCh1, SIGNAL(valueChanged(double)), this, SLOT(slotPrevTrackPlayer1(double)));
    connect(m_pNextTrackCh2, SIGNAL(valueChanged(double)), this, SLOT(slotNextTrackPlayer2(double)));
    connect(m_pPrevTrackCh2, SIGNAL(valueChanged(double)), this, SLOT(slotPrevTrackPlayer2(double)));
    
    TrackPlaylist::setTrack(this);
}

Track::~Track()
{
}

void Track::readXML(QString location)
{
    // Open XML file
    QFile file(location);
    QDomDocument domXML("Mixxx_Track_List");

    // Check if we can open the file
    if (!file.exists())
    {
        //qDebug("Track: %s does not exist.",location.latin1());
        file.close();
        return;
    }

    // Check if there is a parsing problem
    if (!domXML.setContent(&file))
    {
        qDebug("Track: Parse error in %s",location.latin1());
        file.close();
        return;
    }

    file.close();

    // Get the root element
    QDomElement elementRoot = domXML.documentElement();

    // Get version
    //int version = XmlParse::selectNodeInt(elementRoot, "Version");

    // Initialize track collection
    QDomNode node = XmlParse::selectNode(elementRoot, "TrackList");
    m_pTrackCollection->readXML(node);

    // Get all the Playlists written in the xml file:
    node = XmlParse::selectNode(elementRoot, "Playlists").firstChild();
    while (!node.isNull())
    {
        if (node.isElement() && node.nodeName()=="Playlist")
            m_qPlaylists.append(new TrackPlaylist(m_pTrackCollection, node));

        node = node.nextSibling();
    }
}

void Track::writeXML(QString location)
{
    // Create the xml document:
    QDomDocument domXML( "Mixxx_Track_List" );

    // Ensure UTF16 encoding
    domXML.appendChild(domXML.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-16\""));

    // Set the document type
    QDomElement elementRoot = domXML.createElement( "Mixxx_Track_List" );
    domXML.appendChild(elementRoot);

    // Add version information:
    XmlParse::addElement(domXML, elementRoot, "Version", QString("%1").arg(TRACK_VERSION));

    // Write collection of tracks
    m_pTrackCollection->writeXML(domXML, elementRoot);

    // Write playlists
    QDomElement playlistsroot = domXML.createElement("Playlists");

    TrackPlaylist *it = m_qPlaylists.first();
    while (it)
    {
        QDomElement elementNew = domXML.createElement("Playlist");
        it->writeXML(domXML, elementNew);
        playlistsroot.appendChild(elementNew);

        it = m_qPlaylists.next();
    }
    elementRoot.appendChild(playlistsroot);

    // Open the file:
    QFile opmlFile(location);
    if (!opmlFile.open(IO_WriteOnly))
    {
        QMessageBox::critical(0,
                tr("Error"),
                tr("Cannot open file %1").arg(location));
        return;
    }

    // Write to the file:
    QTextStream Xml(&opmlFile);
    Xml.setEncoding(QTextStream::Unicode);
    Xml << domXML.toString();
    opmlFile.close();
}

TrackCollection *Track::getTrackCollection()
{
    return m_pTrackCollection;
}

void Track::slotDrop(QDropEvent *e)
{
    //qDebug("track drop");

    QString name;
#ifndef QT3_SUPPORT
    QCString type("playlist");
    if (!QTextDrag::decode(e, name, type))
#else
    if (!e->mimeData()->hasFormat("playlist"))
#endif
    {
        e->ignore();
        return;
    }

#ifdef QT3_SUPPORT
    name = e->mimeData()->text();
#endif

    e->accept();

    qDebug("name %s",name.latin1());

    slotActivatePlaylist(name);
}

void Track::slotActivatePlaylist(QString name)
{
    // Get pointer to requested playlist
    TrackPlaylist *pNewlist = getPlaylist(name);

    if (pNewlist)
    {
        // Deactivate current playlist
        if (m_pActivePlaylist)
            m_pActivePlaylist->deactivate();

        // Activate new playlist
        m_pActivePlaylist = pNewlist;
        m_pActivePlaylist->activate(m_pView->m_pTrackTable);
        emit(activePlaylist(pNewlist));
    }
}

void Track::slotNewPlaylist()
{
    // Find valid name for new playlist
    int i = 1;
    while (getPlaylist(QString("Default %1").arg(i)))
        ++i;

    TrackPlaylist *p = new TrackPlaylist(m_pTrackCollection, QString("Default %1").arg(i));
    m_qPlaylists.append(p);

    // Make the new playlist active
    slotActivatePlaylist(p->getListName());

    // Update list views
    updatePlaylistViews();
}

void Track::slotDeletePlaylist(QString qName)
{
    // Is set to true if we need to activate another playlist after this one
    // has been removed
    bool bActivateOtherList = false;

    TrackPlaylist *list = getPlaylist(qName);
    if (list)
    {
        // If the deleted list is the active list...
        if (list==m_pActivePlaylist)
        {
            // Deactivate the list
            list->deactivate();
            m_pActivePlaylist = 0;

            bActivateOtherList = true;
        }

        m_qPlaylists.remove(list);
        delete list;
    }

    if (bActivateOtherList)
    {
        if (m_qPlaylists.count()==0)
            slotNewPlaylist();

        slotActivatePlaylist(m_qPlaylists.at(0)->getListName());
    }

    updatePlaylistViews();
}

void Track::slotImportPlaylist()
{
    // Find valid name for new playlist
    int i = 1;
    while (getPlaylist(QString("Imported %1").arg(i)))
        ++i;

    QString sPlsname(QString("Imported %1").arg(i));
    TrackPlaylist * pTempPlaylist = m_pTrackImporter->importPlaylist(sPlsname);

    if (pTempPlaylist != 0)
    {
        m_qPlaylists.append(pTempPlaylist);
    }
    updatePlaylistViews();
}

TrackPlaylist *Track::getPlaylist(QString qName)
{
    TrackPlaylist *it = m_qPlaylists.first();
    while (it)
    {
        if (it->getListName()==qName)
            return it;
        it = m_qPlaylists.next();
    }
    return 0;
}

void Track::slotTrackPopup(TrackInfoObject *pTrackInfoObject, int)
{
    QPopupMenu *menu = new QPopupMenu();

    m_pActivePopupTrack = pTrackInfoObject;

    int id;

    id = menu->insertItem("Player 1", this, SLOT(slotLoadPlayer1()));
    if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()==1.)
        menu->setItemEnabled(id, false);
    
    id = menu->insertItem("Player 2", this, SLOT(slotLoadPlayer2()));
    if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()==1.)
        menu->setItemEnabled(id, false);
    
    menu->insertItem("Remove",   this, SLOT(slotRemoveFromPlaylist()));

    menu->exec(QCursor::pos());

}

void Track::slotLoadPlayer1(TrackInfoObject *pTrackInfoObject, bool bStartFromEndPos)
{
    if (m_pTrackPlayer1)
    {
        m_pTrackPlayer1->setOverviewWidget(0);
        m_pTrackPlayer1->setBpmControlObject(0);
    }

    m_pTrackPlayer1 = pTrackInfoObject;

    // Update score:
    m_pTrackPlayer1->incTimesPlayed();
    if (m_pActivePlaylist)
        m_pActivePlaylist->updateScores();

    // Request a new track from the reader:
    m_pBuffer1->getReader()->requestNewTrack(m_pTrackPlayer1, bStartFromEndPos);

    // Generate waveform summary
    if (m_pTrackPlayer1->getBpm()==0. || (m_pWaveSummary && (m_pTrackPlayer1->getWaveSummary()==0 || m_pTrackPlayer1->getWaveSummary()->size()==0)))
        m_pWaveSummary->enqueue(m_pTrackPlayer1);

    // Set waveform summary display
    m_pTrackPlayer1->setOverviewWidget(m_pView->m_pOverviewCh1);

    // Set control for beat start position for use in EngineTemporal and
    // VisualTemporalBuffer. HACK.
    ControlObject *p = ControlObject::getControl(ConfigKey("[Channel1]","temporalBeatFirst"));
    if (p) 
        p->queueFromThread(m_pTrackPlayer1->getBeatFirst());
    
    // Set Engine file BPM ControlObject
    m_pTrackPlayer1->setBpmControlObject(ControlObject::getControl(ConfigKey("[Channel1]","file_bpm")));    
    
    // Set duration in playpos widget
    if (m_pView->m_pNumberPosCh1)
        m_pView->m_pNumberPosCh1->setDuration(m_pTrackPlayer1->getDuration());

    // Write info to text display
    if (m_pView->m_pTextCh1)
        m_pView->m_pTextCh1->setText(m_pTrackPlayer1->getInfo());

    emit(newTrackPlayer1(m_pTrackPlayer1));
}

void Track::slotLoadPlayer2(TrackInfoObject *pTrackInfoObject, bool bStartFromEndPos)
{
    if (m_pTrackPlayer2)
    {
        m_pTrackPlayer2->setOverviewWidget(0);
        m_pTrackPlayer2->setBpmControlObject(0);
    }

    m_pTrackPlayer2 = pTrackInfoObject;

    // Update score:
    m_pTrackPlayer2->incTimesPlayed();
    if (m_pActivePlaylist)
        m_pActivePlaylist->updateScores();

    // Request a new track from the reader:
    m_pBuffer2->getReader()->requestNewTrack(m_pTrackPlayer2, bStartFromEndPos);

    // Generate waveform summary
    if (m_pTrackPlayer2->getBpm()==0. || (m_pWaveSummary && (m_pTrackPlayer2->getWaveSummary()==0 || m_pTrackPlayer2->getWaveSummary()->size()==0)))
        m_pWaveSummary->enqueue(m_pTrackPlayer2);

    // Set waveform summary display
    m_pTrackPlayer2->setOverviewWidget(m_pView->m_pOverviewCh2);

    // Set control for beat start position for use in EngineTemporal and
    // VisualTemporalBuffer. HACK.
    ControlObject *p = ControlObject::getControl(ConfigKey("[Channel2]","temporalBeatFirst"));
    if (p) 
        p->queueFromThread(m_pTrackPlayer2->getBeatFirst());
    
    // Set Engine file BPM ControlObject
    m_pTrackPlayer2->setBpmControlObject(ControlObject::getControl(ConfigKey("[Channel2]","file_bpm")));    
    
    // Set duration in playpos widget
    if (m_pView->m_pNumberPosCh2)
        m_pView->m_pNumberPosCh2->setDuration(m_pTrackPlayer2->getDuration());

    // Write info to text display
    if (m_pView->m_pTextCh2)
        m_pView->m_pTextCh2->setText(m_pTrackPlayer2->getInfo());

    emit(newTrackPlayer2(m_pTrackPlayer2));
}

void Track::slotLoadPlayer1()
{
    slotLoadPlayer1(m_pActivePopupTrack);
}

void Track::slotLoadPlayer2()
{
    slotLoadPlayer2(m_pActivePopupTrack);
}

void Track::slotLoadPlayer1(QString filename)
{
    TrackInfoObject *pTrack = m_pTrackCollection->getTrack(filename);
    if (pTrack)
        slotLoadPlayer1(pTrack);
}

void Track::slotLoadPlayer2(QString filename)
{
    TrackInfoObject *pTrack = m_pTrackCollection->getTrack(filename);
    if (pTrack)
        slotLoadPlayer2(pTrack);
}

TrackPlaylist *Track::getActivePlaylist()
{
    return m_pActivePlaylist;
}

void Track::slotRemoveFromPlaylist()
{
    m_pActivePlaylist->slotRemoveTrack(m_pActivePopupTrack);
}

void Track::slotEndOfTrackPlayer1(double val)
{
//    qDebug("end of track %f",val);
    
    if (val==0.)
        return; 
        
    switch ((int)m_pEndOfTrackModeCh1->get())
    {
    case TRACK_END_MODE_NEXT:
        if (m_pTrackPlayer1)
        {
            TrackInfoObject *pTrack;
            bool bStartFromEndPos = false;
            
            if (m_pPlayPositionCh1->get()>0.5)
                pTrack = m_pTrackPlayer1->getNext();
            else
            {
                pTrack = m_pTrackPlayer1->getPrev();
                bStartFromEndPos = true;
            }

            if (bStartFromEndPos)
                qDebug("start from end");
                        
            if (pTrack)
                slotLoadPlayer1(pTrack, bStartFromEndPos);
            else
            {
//                 m_pPlayButtonCh1->slotSet(0.);
                m_pEndOfTrackCh1->slotSet(0.);
            }
        }
        break;
    }
    //m_pEndOfTrackCh1->slotSet(0.);
}

void Track::slotEndOfTrackPlayer2(double val)
{
    if (val==0.)
        return; 

    switch ((int)m_pEndOfTrackModeCh2->get())
    {
    case TRACK_END_MODE_NEXT:
        if (m_pTrackPlayer2)
        {
            TrackInfoObject *pTrack;
            bool bStartFromEndPos = false;
            
            if (m_pPlayPositionCh2->get()>0.5)
                pTrack = m_pTrackPlayer2->getNext();
            else
            {
                pTrack = m_pTrackPlayer2->getPrev();
                bStartFromEndPos = true;
            }
                
            if (pTrack)
                slotLoadPlayer2(pTrack, bStartFromEndPos);
            else
            {    
//                 m_pPlayButtonCh2->slotSet(0.);
                m_pEndOfTrackCh2->slotSet(0.);
            }
        }
        break;
    }
    //m_pEndOfTrackCh2->slotSet(0.);
}

void Track::slotNextTrackPlayer1(double v)
{
    if (v && m_pTrackPlayer1)
    {
        TrackInfoObject *pTrack = m_pTrackPlayer1->getNext();
        if (pTrack)
            slotLoadPlayer1(pTrack);
    }
}

void Track::slotPrevTrackPlayer1(double v)
{
    if (v && m_pTrackPlayer1)
    {
        TrackInfoObject *pTrack = m_pTrackPlayer1->getPrev();
        if (pTrack)
            slotLoadPlayer1(pTrack);
    }
}

void Track::slotNextTrackPlayer2(double v)
{
    if (v && m_pTrackPlayer2)
    {
        TrackInfoObject *pTrack = m_pTrackPlayer2->getNext();
        if (pTrack)
            slotLoadPlayer2(pTrack);
    }
}

void Track::slotPrevTrackPlayer2(double v)
{
    if (v && m_pTrackPlayer2)
    {
        TrackInfoObject *pTrack = m_pTrackPlayer2->getPrev();
        if (pTrack)
            slotLoadPlayer2(pTrack);
    }
}

void Track::updatePlaylistViews()
{
    // Sort list
    m_qPlaylists.sort();

    // Update tree view
    if (m_pView->m_pTreeView)
        m_pView->m_pTreeView->updatePlaylists(&m_qPlaylists);

    // Update menu
    emit(updateMenu(&m_qPlaylists));

    // Set active
    if (m_pActivePlaylist)
        emit(activePlaylist(m_pActivePlaylist));
}
