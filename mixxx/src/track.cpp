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

Track::Track(QString location, MixxxView *pView, EngineBuffer *pBuffer1, EngineBuffer *pBuffer2)
{
    m_pView = pView;
    m_pBuffer1 = pBuffer1;
    m_pBuffer2 = pBuffer2;
    m_pActivePlaylist = 0;
    m_pActivePopupPlaylist = 0;
    m_pTrackPlayer1 = 0;
    m_pTrackPlayer2 = 0;

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
        qDebug("Track: %s does not exists.",location.latin1());
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
    domXML.appendChild(domXML.createProcessingInstruction("xml","version=\" 1.0 \" encoding=\"UTF-16\""));

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
    QCString type("playlist");
    if (!QTextDrag::decode(e, name, type))
    {
        e->ignore();
        return;
    }

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

    menu->insertItem("Player 1", this, SLOT(slotLoadPlayer1()));
    menu->insertItem("Player 2", this, SLOT(slotLoadPlayer2()));
    menu->insertItem("Remove",   this, SLOT(slotRemoveFromPlaylist()));

    menu->exec(QCursor::pos());

}

void Track::slotLoadPlayer1(TrackInfoObject *pTrackInfoObject)
{
    m_pTrackPlayer1 = pTrackInfoObject;
    emit(newTrackPlayer1(m_pTrackPlayer1));

    // Update score:
    m_pTrackPlayer1->incTimesPlayed();
    if (m_pActivePlaylist)
        m_pActivePlaylist->updateScores();

    // Request a new track from the reader:
    m_pBuffer1->getReader()->requestNewTrack(m_pTrackPlayer1);

    // Set duration in playpos widget
    if (m_pView->m_pNumberPosCh1)
        m_pView->m_pNumberPosCh1->setDuration(m_pTrackPlayer1->getDuration());

    // Write info to text display
    if (m_pView->m_pTextCh1)
        m_pView->m_pTextCh1->setText(m_pTrackPlayer1->getInfo());
}

void Track::slotLoadPlayer2(TrackInfoObject *pTrackInfoObject)
{
    m_pTrackPlayer2 = pTrackInfoObject;
    emit(newTrackPlayer2(m_pTrackPlayer2));

    // Update score:
    m_pTrackPlayer2->incTimesPlayed();
    if (m_pActivePlaylist)
        m_pActivePlaylist->updateScores();

    // Request a new track from the reader:
    m_pBuffer2->getReader()->requestNewTrack(m_pTrackPlayer2);

    // Set duration in playpos widget
    if (m_pView->m_pNumberPosCh2)
        m_pView->m_pNumberPosCh2->setDuration(m_pTrackPlayer2->getDuration());

    // Write info to text display
    if (m_pView->m_pTextCh2)
        m_pView->m_pTextCh2->setText(m_pTrackPlayer2->getInfo());
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

void Track::slotRemoveFromPlaylist()
{
    m_pActivePlaylist->slotRemoveTrack(m_pActivePopupTrack);
}

void Track::slotEndOfTrackPlayer1(double)
{
    switch ((int)m_pEndOfTrackModeCh1->get())
    {
    case TRACK_END_MODE_NEXT:
        if (m_pTrackPlayer1)
        {
            TrackInfoObject *pTrack = m_pTrackPlayer1->getNext();
            if (pTrack)
                slotLoadPlayer1(pTrack);
            else
                m_pPlayButtonCh1->slotSet(0.);
        }
        break;
    }
    m_pEndOfTrackCh1->slotSet(0.);
}

void Track::slotEndOfTrackPlayer2(double)
{
    switch ((int)m_pEndOfTrackModeCh2->get())
    {
    case TRACK_END_MODE_NEXT:
        if (m_pTrackPlayer2)
        {
            TrackInfoObject *pTrack = m_pTrackPlayer2->getNext();
            if (pTrack)
                slotLoadPlayer2(pTrack);
            else
                m_pPlayButtonCh2->slotSet(0.);
        }
        break;
    }
    m_pEndOfTrackCh2->slotSet(0.);
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
