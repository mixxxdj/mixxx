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
#include <QLabel>
#include <qcombobox.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <QDropEvent>
#include "mixxxview.h"
#include <q3dragobject.h>
#include "wtracktable.h"
/*used for new model/view interface*/
#include "wtracktablemodel.h"
#include "wtracktableview.h"

#include "wtreeview.h"
#include "dlgbpmtap.h"
#include "wnumberpos.h"
#include <q3popupmenu.h>
#include <qcursor.h>
#include <q3cstring.h>
#include "enginebuffer.h"
#include "reader.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "configobject.h"
#include "trackimporter.h"
#include "wavesummary.h"
#include "bpmdetector.h"
#include "woverview.h"
#include <q3progressdialog.h>

Track::Track(QString location, MixxxView * pView, EngineBuffer * pBuffer1, EngineBuffer * pBuffer2, WaveSummary * pWaveSummary, BpmDetector * pBpmDetector, QString musiclocation)
{
    m_pView = pView;
    m_pBuffer1 = pBuffer1;
    m_pBuffer2 = pBuffer2;
    m_pActivePlaylist = 0;
    m_pActivePopupPlaylist = 0;
    m_pTrackPlayer1 = 0;
    m_pTrackPlayer2 = 0;
    m_pWaveSummary = pWaveSummary;
    m_pBpmDetector = pBpmDetector;

    musicDir = musiclocation;

    m_pTrackCollection = new TrackCollection(m_pBpmDetector);
    m_pTrackImporter = new TrackImporter(m_pView,m_pTrackCollection);
    m_pLibraryModel = new WTrackTableModel(m_pView->m_pTrackTableView);
    m_pPlayQueueModel = new WTrackTableModel(m_pView->m_pTrackTableView);

    // Read the XML file
    readXML(location);

    // Ensure that two playlists are present
    if(m_qPlaylists.count() < 2)
    {
        for(int i = m_qPlaylists.count(); i<2; ++i)
        {
            m_qPlaylists.append(new TrackPlaylist(m_pTrackCollection,(QString("Default %1").arg(i))));
        }
        m_qPlaylists.at(0)->addPath(musicDir);
    }


    // Update tree view
    //updatePlaylistViews();

    // Insert the first playlist in the list
    m_pActivePlaylist = m_qPlaylists.at(0);
    //m_pActivePlaylist->activate(m_pView->m_pTrackTable);

    m_pLibraryModel->setTrackPlaylist(m_qPlaylists.at(0));
    m_pPlayQueueModel->setTrackPlaylist(m_qPlaylists.at(1));

    if (m_pView) //Stops Mixxx from dying if a skin doesn't have the search box.
    {
        m_pView->m_pTrackTableView->setSearchSource(m_pLibraryModel);
        m_pView->m_pTrackTableView->resizeColumnsToContents();
        m_pView->m_pTrackTableView->setTrack(this);

        // Connect ComboBox events to WTrackTable
        connect(m_pView->m_pComboBox, SIGNAL(activated(int)), this, SLOT(slotActivatePlaylist(int)));

        // Connect Search to table
        connect( m_pView->m_pLineEditSearch,
                SIGNAL( textChanged( const QString & )),
                m_pView->m_pTrackTableView->m_pSearchFilter,
                SLOT( setFilterFixedString( const QString & )));

        connect( m_pView->m_pLineEditSearch,
                SIGNAL( textChanged( const QString & )),
                m_pView->m_pTrackTableView->m_pDirFilter,
                SLOT( setFilterFixedString( const QString & )));

        // Connect drop events to table
        //connect(m_pView->m_pTrackTable, SIGNAL(dropped(QDropEvent *)), this, SLOT(slotDrop(QDropEvent *)));
    }


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
        //qDebug("Track: Parse error in %s",location.latin1());
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

    //qDebug("Break");

    // Get all the Playlists written in the xml file:
    node = XmlParse::selectNode(elementRoot, "Playlists").firstChild();
    while (!node.isNull())
    {
        if (node.isElement() && node.nodeName()=="Playlist")
            m_qPlaylists.append(new TrackPlaylist(m_pTrackCollection, node));

        node = node.nextSibling();
    }
    //CTAF TODO: TEMPORARY REMOVED, NEED TO BE DONE IN A THREAD
    if(m_qPlaylists.count() >= 2)
        m_qPlaylists.at(0)->addPath(musicDir);
    //ECTAF
}

void Track::writeXML(QString location)
{
    Q3ProgressDialog progress( "Writing song database...", 0, m_qPlaylists.count()+5,
                              0, "progress", TRUE );
    progress.show();
    int i = 0;

    // Create the xml document:
    QDomDocument domXML( "Mixxx_Track_List" );

    // Ensure UTF16 encoding
    domXML.appendChild(domXML.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-16\""));

    // Set the document type
    QDomElement elementRoot = domXML.createElement( "Mixxx_Track_List" );
    domXML.appendChild(elementRoot);

    // Add version information:
    XmlParse::addElement(domXML, elementRoot, "Version", QString("%1").arg(TRACK_VERSION));

    progress.setProgress(++i);
    qApp->processEvents();

    // Write collection of tracks
    m_pTrackCollection->writeXML(domXML, elementRoot);

    progress.setProgress(++i);
    qApp->processEvents();

    // Write playlists
    QDomElement playlistsroot = domXML.createElement("Playlists");

    TrackPlaylist * it = m_qPlaylists.first();
    while (it)
    {
        progress.setProgress(++i);
        qApp->processEvents();

        QDomElement elementNew = domXML.createElement("Playlist");
        it->writeXML(domXML, elementNew);
        playlistsroot.appendChild(elementNew);

        it = m_qPlaylists.next();

    }
    elementRoot.appendChild(playlistsroot);

    progress.setProgress(++i);
    qApp->processEvents();

    // Open the file:
    QFile opmlFile(location);
    if (!opmlFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(0,
                              tr("Error"),
                              tr("Cannot open file %1").arg(location));
        return;
    }

    progress.setProgress(++i);
    qApp->processEvents();

    // Write to the file:
    Q3TextStream Xml(&opmlFile);
    Xml.setEncoding(Q3TextStream::Unicode);
    Xml << domXML.toString();
    opmlFile.close();

    progress.setProgress(++i);
    qApp->processEvents();

}

TrackCollection * Track::getTrackCollection()
{
    return m_pTrackCollection;
}

void Track::slotDrop(QDropEvent * e)
{
    qDebug("track drop");

    QString name;
#ifndef QT3_SUPPORT
    Q3CString type("playlist");
    if (!Q3TextDrag::decode(e, name, type))
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


/* CTAF: I dont think this function is used actually */
void Track::slotActivatePlaylist(QString name)
{
    qDebug("playlist change\n");
    // Get pointer to requested playlist
    TrackPlaylist * pNewlist = getPlaylist(name);

    if (pNewlist)
    {
        // Deactivate current playlist
        //if (m_pActivePlaylist)
        //m_pActivePlaylist->deactivate();
        // Activate new playlist
        m_pActivePlaylist = pNewlist;
        m_pLibraryModel->setTrackPlaylist(m_pActivePlaylist);
        //m_pActivePlaylist->activate(m_pView->m_pTrackTable);
        emit(activePlaylist(pNewlist));
    }
}


void Track::slotActivatePlaylist(int index)
{
    switch(index)
    {
    case 0:
        m_pView->m_pTrackTableView->reset();
        m_pView->m_pTrackTableView->setSearchSource(m_pLibraryModel);
        m_pView->m_pTrackTableView->resizeColumnsToContents();
        m_pView->m_pTrackTableView->setTrack(this);
        break;
    case 1:
        m_pView->m_pTrackTableView->reset();
        m_pView->m_pTrackTableView->setSearchSource(m_pPlayQueueModel);
        m_pView->m_pTrackTableView->resizeColumnsToContents();
        m_pView->m_pTrackTableView->setTrack(this);
        break;
    case 2:
        m_pView->m_pTrackTableView->reset();
        m_pView->m_pTrackTableView->setDirModel();
        m_pView->m_pTrackTableView->resizeColumnsToContents();
        m_pView->m_pTrackTableView->setTrack(this);
    }
    //if (m_pActivePlaylist)
    //m_pActivePlaylist->deactivate();

    // Insert playlist according to ComboBox index
    m_pActivePlaylist = m_qPlaylists.at(index);
    //m_pActivePlaylist->activate(m_pView->m_pTrackTable);
}
void Track::slotNewPlaylist()
{
    // Find valid name for new playlist
    int i = 1;
    while (getPlaylist(QString("Default %1").arg(i)))
        ++i;

    TrackPlaylist * p = new TrackPlaylist(m_pTrackCollection, QString("Default %1").arg(i));
    m_qPlaylists.append(p);

    // Make the new playlist active
    //slotActivatePlaylist(p->getListName());

    // Update list views
    //updatePlaylistViews();
}

void Track::slotDeletePlaylist(QString qName)
{
    // Is set to true if we need to activate another playlist after this one
    // has been removed
    bool bActivateOtherList = false;

    TrackPlaylist * list = getPlaylist(qName);
    if (list)
    {
        // If the deleted list is the active list...
        if (list==m_pActivePlaylist)
        {
            // Deactivate the list
            //list->deactivate();
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

    //updatePlaylistViews();
}

void Track::slotImportPlaylist()
{
    // Find valid name for new playlist
    int i = 1;
    while (getPlaylist(QString("Imported %1").arg(i)))
        ++i;

    QString sPlsname(QString("Imported %1").arg(i));
    TrackPlaylist * pTempPlaylist = m_pTrackImporter->importPlaylist(sPlsname);

    if (pTempPlaylist != NULL)
    {
        m_qPlaylists.append(pTempPlaylist);
    }
    //updatePlaylistViews();
}

TrackPlaylist * Track::getPlaylist(QString qName)
{
    TrackPlaylist * it = m_qPlaylists.first();
    while (it)
    {
        if (it->getListName()==qName)
            return it;
        it = m_qPlaylists.next();
    }
    return 0;
}

void Track::slotSendToPlayqueue(TrackInfoObject * pTrackInfoObject)
{
    m_qPlaylists.at(1)->addTrack(pTrackInfoObject);
}

void Track::slotSendToPlayqueue(QString filename)
{
    TrackInfoObject* pTrack = m_pTrackCollection->getTrack(filename);
    if (pTrack)
        m_qPlaylists.at(1)->addTrack(pTrack);
}

void Track::slotLoadPlayer1(TrackInfoObject * pTrackInfoObject, bool bStartFromEndPos)
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

    // Detect BPM if required
    if (m_pTrackPlayer1->getBpmConfirm()== false || m_pTrackPlayer1->getBpm() == 0.)
        m_pTrackPlayer1->sendToBpmQueue();

    // Generate waveform summary
    if ((m_pWaveSummary && (m_pTrackPlayer1->getWaveSummary()==0 || m_pTrackPlayer1->getWaveSummary()->size()==0)))
        m_pWaveSummary->enqueue(m_pTrackPlayer1);

    // Set waveform summary display
    m_pTrackPlayer1->setOverviewWidget(m_pView->m_pOverviewCh1);

    // Set control for beat start position for use in EngineTemporal and
    // VisualTemporalBuffer. HACK.
    ControlObject * p = ControlObject::getControl(ConfigKey("[Channel1]","temporalBeatFirst"));
    if (p)
        p->queueFromThread(m_pTrackPlayer1->getBeatFirst());

    // Set Engine file BPM and duration ControlObjects
    m_pTrackPlayer1->setBpmControlObject(ControlObject::getControl(ConfigKey("[Channel1]","file_bpm")));
    m_pTrackPlayer1->setDurationControlObject(ControlObject::getControl(ConfigKey("[Channel1]","duration")));

    // Set duration in playpos widget
//    if (m_pView->m_pNumberPosCh1)
//        m_pView->m_pNumberPosCh1->setDuration(m_pTrackPlayer1->getDuration());

    // Write info to text display
    if (m_pView->m_pTextCh1)
        m_pView->m_pTextCh1->setText(m_pTrackPlayer1->getInfo());

    emit(newTrackPlayer1(m_pTrackPlayer1));
}

void Track::slotLoadPlayer2(TrackInfoObject * pTrackInfoObject, bool bStartFromEndPos)
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

    // Detect BPM if required
    if (m_pTrackPlayer2->getBpmConfirm()== false || m_pTrackPlayer2->getBpm() == 0.)
        m_pTrackPlayer2->sendToBpmQueue();

    // Generate waveform summary
    if ((m_pWaveSummary && (m_pTrackPlayer2->getWaveSummary()==0 || m_pTrackPlayer2->getWaveSummary()->size()==0)))
        m_pWaveSummary->enqueue(m_pTrackPlayer2);

    // Set waveform summary display
    m_pTrackPlayer2->setOverviewWidget(m_pView->m_pOverviewCh2);
    m_pTrackPlayer2->setDurationControlObject(ControlObject::getControl(ConfigKey("[Channel2]","duration")));

    // Set control for beat start position for use in EngineTemporal and
    // VisualTemporalBuffer. HACK.
    ControlObject * p = ControlObject::getControl(ConfigKey("[Channel2]","temporalBeatFirst"));
    if (p)
        p->queueFromThread(m_pTrackPlayer2->getBeatFirst());

    // Set Engine file BPM ControlObject
    m_pTrackPlayer2->setBpmControlObject(ControlObject::getControl(ConfigKey("[Channel2]","file_bpm")));

    // Set duration in playpos widget
//    if (m_pView->m_pNumberPosCh2)
//        m_pView->m_pNumberPosCh2->setDuration(m_pTrackPlayer2->getDuration());

    // Write info to text display
    if (m_pView->m_pTextCh2)
        m_pView->m_pTextCh2->setText(m_pTrackPlayer2->getInfo());

    emit(newTrackPlayer2(m_pTrackPlayer2));
}

void Track::slotLoadPlayer1(QString filename)
{
    TrackInfoObject * pTrack = m_pTrackCollection->getTrack(filename);
    if (pTrack)
        slotLoadPlayer1(pTrack);
}

void Track::slotLoadPlayer2(QString filename)
{
    TrackInfoObject * pTrack = m_pTrackCollection->getTrack(filename);
    if (pTrack)
        slotLoadPlayer2(pTrack);
}

TrackPlaylist * Track::getActivePlaylist()
{
    return m_pActivePlaylist;
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
            TrackInfoObject * pTrack;
            bool bStartFromEndPos = false;
            
            //Load the next song...
            if (m_pPlayPositionCh1->get()>0.5)
            {
               //If the play queue has another song in it, load that...
               pTrack = m_pTrackPlayer1->getNext(m_qPlaylists.at(1));
               //Otherwise load from the active playlist... //FIXME
               if (!pTrack)
                    pTrack = m_pTrackPlayer1->getNext(m_pActivePlaylist);
                
            }
            else
            {
                pTrack = m_pTrackPlayer1->getPrev(m_pActivePlaylist);
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
            TrackInfoObject * pTrack;
            bool bStartFromEndPos = false;

            if (m_pPlayPositionCh2->get()>0.5)
                pTrack = m_pTrackPlayer2->getNext(m_pActivePlaylist);
            else
            {
                pTrack = m_pTrackPlayer2->getPrev(m_pActivePlaylist);
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
        TrackInfoObject * pTrack = m_pTrackPlayer1->getNext(m_pActivePlaylist);
        if (pTrack)
            slotLoadPlayer1(pTrack);
    }
}

void Track::slotPrevTrackPlayer1(double v)
{
    if (v && m_pTrackPlayer1)
    {
        TrackInfoObject * pTrack = m_pTrackPlayer1->getPrev(m_pActivePlaylist);
        if (pTrack)
            slotLoadPlayer1(pTrack);
    }
}

void Track::slotNextTrackPlayer2(double v)
{
    if (v && m_pTrackPlayer2)
    {
        TrackInfoObject * pTrack = m_pTrackPlayer2->getNext(m_pActivePlaylist);
        if (pTrack)
            slotLoadPlayer2(pTrack);
    }
}

void Track::slotPrevTrackPlayer2(double v)
{
    if (v && m_pTrackPlayer2)
    {
        TrackInfoObject * pTrack = m_pTrackPlayer2->getPrev(m_pActivePlaylist);
        if (pTrack)
            slotLoadPlayer2(pTrack);
    }
}

/**void Track::updatePlaylistViews()
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
   }*/

