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

#ifdef __WINDOWS__
#include <windows.h> // for Sleep()  on Windows
#endif

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

#include "cachingreader.h"

/*used for new model/view interface*/
#include "wtracktablemodel.h"
#include "wplaylistlistmodel.h"
#include "wpromotracksmodel.h"

#ifdef __IPOD__
#include "wipodtracksmodel.h"
#endif

#include "wtracktableview.h"
#include "libraryscanner.h"
#include "libraryscannerdlg.h"

//#include "wtreeview.h"
#include "dlgbpmtap.h"
#include "widget/wnumberpos.h"
#include <QMenu>
#include <QCursor>
#include <q3cstring.h>
#include "engine/enginebuffer.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "configobject.h"
#include "analyserqueue.h"
#include "trackimporter.h"
#include "playerinfo.h"
#include "defs_promo.h"
#include "soundsourceproxy.h"

#include <q3progressdialog.h>

#include <QEvent>
#include <QObject>
#include <QLineEdit>
#include <QDebug>

// Include widget object types for eventFilter
#include "widget/wslidercomposed.h"
#include "widget/wknob.h"
#include "widget/woverview.h"
//#include "widget/wpushButton.h"
#include "widget/wwidget.h"

#ifdef __C_METRICS__
#include <cmetrics.h>
#include "defs_mixxxcmetrics.h"
#endif

Track::Track(QString location, MixxxView * pView, ConfigObject<ConfigValue> *config, 
			 EngineBuffer * pBuffer1, EngineBuffer * pBuffer2,
			 AnalyserQueue* analyserQueue)
{
    m_pView = pView;
    m_pBuffer1 = pBuffer1;
    m_pBuffer2 = pBuffer2;
    m_pActivePlaylist = 0;
    m_pActivePopupPlaylist = 0;
    m_pTrackPlayer1 = 0;
    m_pTrackPlayer2 = 0;
    m_pConfig = config;
    m_iLibraryIdx = 0;   //FIXME: Deprecated, can safely remove.
    m_iPlayqueueIdx = 0; //FIXME: Deprecated, can safely remove.

	m_analyserQueue = analyserQueue;

    m_pTrackCollection = new TrackCollection();
    m_pTrackImporter = new TrackImporter(m_pView,m_pTrackCollection);
    m_pLibraryModel = new WTrackTableModel(m_pView->m_pTrackTableView);
    m_pPlayQueueModel = new WTrackTableModel(m_pView->m_pTrackTableView);
    m_pPromoModel = new WPromoTracksModel(m_pView->m_pTrackTableView);
#ifdef __IPOD__
    m_pIPodModel = new WIPodTracksModel(m_pView->m_pTrackTableView);
#endif
    m_pPlaylistModel = new WTrackTableModel(m_pView->m_pTrackTableView);
    m_pPlaylistListModel = new WPlaylistListModel(m_pView->m_pTrackTableView);

    //Pass the track collcetion along to the library and playqueue playlists.
    m_qLibraryPlaylist.setTrackCollection(m_pTrackCollection);
    m_qPlayqueuePlaylist.setTrackCollection(m_pTrackCollection);
    m_qPromoPlaylist.setTrackCollection(m_pTrackCollection);

	m_timerID = 0;

#ifdef __IPOD__
    m_qIPodPlaylist.setTrackCollection(m_pTrackCollection);
    m_qIPodPlaylist.setListName(PLAYLIST_NAME_IPOD);
#endif

   // Read the XML file
    qDebug() << "Loading playlists and library tracks from XML...";
    readXML(location);

    //Initialize the promo tracks playlist
    initPromoTracks();

    if (m_pView && m_pView->m_pTrackTableView) //Stops Mixxx from dying if a skin is from Mixxx <= 1.5.0.
    {
        m_pScanner = new LibraryScanner(&m_qLibraryPlaylist, "");
        //Refresh the tableview when the library is done being scanned. (FIXME: Is a hack)
        connect(m_pScanner, SIGNAL(scanFinished()), m_pView->m_pTrackTableView, SLOT(repaintEverything()));

        //The WTrackTableView doesn't have a pointer to us yet (which it needs to retrieve the list of playlists),
        //so give it one.
        m_pView->m_pTrackTableView->setTrack(this);

        // Update anything that views the playlists
        updatePlaylistViews();

        // Insert the first playlist in the list
        m_pActivePlaylist = &m_qLibraryPlaylist;
        //m_pActivePlaylist->activate(m_pView->m_pTrackTable);

        m_pLibraryModel->setTrackPlaylist(&m_qLibraryPlaylist);
        m_pPlayQueueModel->setTrackPlaylist(&m_qPlayqueuePlaylist);
        m_pPlaylistListModel->setPlaylistList(&m_qPlaylists);
        m_pPromoModel->setTrackPlaylist(&m_qPromoPlaylist);

#ifdef __IPOD__
         m_pIPodModel->setTrackPlaylist(&m_qIPodPlaylist);
#endif

        //If the TrackCollection appears to be empty (could be first run), then scan the library
        if (m_pTrackCollection->getSize() == 0)
        {
            slotScanLibrary();
        }
        else if (checkLibraryLastModified() == true) //Check to see if the library has been modified since
                                                     //we last scanned it.
        {
            slotScanLibrary();
        }

        qDebug() << "Trying to add" << m_pTrackCollection->getSize() << "songs to the library playlist";
        for (int i = 0; i < m_pTrackCollection->getSize(); i++)
        {
            m_qLibraryPlaylist.addTrack(m_pTrackCollection->getTrack(i));
        }

        m_pView->m_pTrackTableView->setSearchSource(m_pLibraryModel);

        // m_pView->m_pTrackTableView->resizeColumnsToContents();
        resizeColumnsForLibraryMode();

        m_pView->m_pTrackTableView->setTrack(this);

        // Connect ComboBox events to WTrackTable
        connect(m_pView->m_pComboBox, SIGNAL(activated(int)), this, SLOT(slotActivatePlaylist(int)));

        // Connect Search to table
/*
        connect( m_pView->m_pLineEditSearch,
                SIGNAL( textChanged( const QString & )),
		m_pView->m_pTrackTableView,
		SLOT(slotFilter(const QString &)));
*/

        savedRowPosition = 0;
        m_timerID = startTimer(250);   // Update the TrackTableView filter a maximum of 4 times a second.

	// add EventFilter to do a selectAll text when the LineEditSearch gains focus
	m_pView->m_pLineEditSearch-> installEventFilter(this);
	m_pView->installEventFilter(this);

        // Connect drop events to table
        //connect(m_pView->m_pTrackTable, SIGNAL(dropped(QDropEvent *)), this, SLOT(slotDrop(QDropEvent *)));
    }
    else
    {
        //If there was no TrackTableView specified in the skin, give a warning.
        QMessageBox::warning(NULL, tr("Mixxx"),
                             QString(tr("You're using a skin that is incompatible with Mixxx %1, which "
                             "will cause unexpected behaviour (eg. missing library).\nThis can happen if you're "
                             "using a third-party skin or if you've incorrectly upgraded Mixxx.")).arg(VERSION),
                             QMessageBox::Ok, QMessageBox::Ok);

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

    // Get cue points 
    m_pCuePointCh1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","cue_point")));
    m_pCuePointCh2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]","cue_point")));


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

    // Make controls for tracklist navigation and current track loading
    m_pLoadSelectedTrackCh1 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel1]","LoadSelectedTrack")));
    m_pLoadSelectedTrackCh2 = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Channel2]","LoadSelectedTrack")));
    m_pSelectNextTrack = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectNextTrack")));
    m_pSelectPrevTrack = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectPrevTrack")));
    m_pLoadSelectedIntoFirstStopped = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","LoadSelectedIntoFirstStopped")));
    m_pSelectTrackKnob = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectTrackKnob")));
    m_pSelectNextPlaylist = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectNextPlaylist")));
    m_pSelectPrevPlaylist = new ControlObjectThreadMain(new ControlObject(ConfigKey("[Playlist]","SelectPrevPlaylist")));

    connect(m_pLoadSelectedTrackCh1, SIGNAL(valueChanged(double)), this, SLOT(slotLoadSelectedTrackCh1(double)));
    connect(m_pLoadSelectedTrackCh2, SIGNAL(valueChanged(double)), this, SLOT(slotLoadSelectedTrackCh2(double)));
    connect(m_pSelectNextTrack, SIGNAL(valueChanged(double)), this, SLOT(slotSelectNextTrack(double)));
    connect(m_pSelectPrevTrack, SIGNAL(valueChanged(double)), this, SLOT(slotSelectPrevTrack(double)));

    connect(m_pSelectNextPlaylist, SIGNAL(valueChanged(double)), this, SLOT(slotSelectNextPlaylist(double)));
    connect(m_pSelectPrevPlaylist, SIGNAL(valueChanged(double)), this, SLOT(slotSelectPrevPlaylist(double)));

    m_pVisualResampleCh1 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel1]","VisualResample")));
    m_pVisualResampleCh2 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Channel2]","VisualResample")));

    connect(m_pLoadSelectedIntoFirstStopped, SIGNAL(valueChanged(double)), this, SLOT(slotLoadSelectedIntoFirstStopped(double)));
    connect(m_pSelectTrackKnob, SIGNAL(valueChanged(double)), this, SLOT(slotSelectTrackKnob(double)));

    TrackPlaylist::setTrack(this);


    connect(m_pBuffer1, SIGNAL(trackLoaded(TrackInfoObject*)),
            this, SLOT(slotFinishLoadingPlayer1(TrackInfoObject*)));
    connect(m_pBuffer2, SIGNAL(trackLoaded(TrackInfoObject*)),
            this, SLOT(slotFinishLoadingPlayer2(TrackInfoObject*)));

	m_pView->m_pTrackTableView->repaintEverything();

}

Track::~Track()
{
}

void Track::appShuttingDown() {
	
	if (m_timerID != 0) {
		killTimer(m_timerID);
	}
}

void Track::resizeColumnsForLibraryMode()
{
        double centa = m_pView->m_pTrackTableView->size().width()/100.;
        qDebug() << "Adjusting column widths: tracktable width =" << m_pView->m_pTrackTableView->size().width() <<" 1% of that is:"<< centa << " FIXME: this should be done when initalizing the skin.";
        m_pView->m_pTrackTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::ARTIST, 15 * centa);
        m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::TITLE, 40 * centa);
        m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::TYPE, (20/4.) * centa);
        m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::LENGTH, (20/4.) * centa);
        m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::BITRATE, (20/4.) * centa);
        m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::BPM, (20/4.) * centa);
        m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::COMMENT, 25 * centa);
        if ( (20/4.) * centa <= 42 ) { // If we won't get enough percentage to display length, we have to make some adjustments...
          qDebug() << "Shrinking Title/Comment for small screen... ";
          m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::TYPE, 35);
          m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::LENGTH, 42);
          m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::BITRATE, 33);
          m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::BPM, 40);
          m_pView->m_pTrackTableView->setColumnWidth(WTrackTableModel::TITLE, (60 * centa) - (35+42+33+40));
        }
}

void Track::timerEvent(QTimerEvent *event) {
  if (m_pView && m_pView->m_pTrackTableView && m_pView->m_pLineEditSearch && m_pView->m_pLineEditSearch->isModified()) {

      if (m_pView->m_pTrackTableView->getTableMode() == TABLE_MODE_LIBRARY && m_pView->m_pLineEditSearch->text() == "") {
      // qDebug() << "Restore viewport to position to row:" << savedRowPosition;
      m_pView->m_pTrackTableView->slotFilter(""); // set the library filter
      m_pView->m_pTrackTableView->scrollTo(m_pView->m_pTrackTableView->model()->index(savedRowPosition,0), QAbstractItemView::PositionAtTop);
    } else if (m_pView->m_pTrackTableView->getTableMode() == TABLE_MODE_LIBRARY && m_pView->m_pTrackTableView->getFilterString() == "") {
      // qDebug() << "save viewport position. Top row:" << m_pView->m_pTrackTableView->rowAt( 1 );
      savedRowPosition = m_pView->m_pTrackTableView->rowAt( 1 );
      m_pView->m_pTrackTableView->slotFilter(m_pView->m_pLineEditSearch->text()); // set the library filter
    } else {
      // qDebug() << "Update filter only. savedRowPosition:" << savedRowPosition;
      m_pView->m_pTrackTableView->slotFilter(m_pView->m_pLineEditSearch->text()); // set the library filter
    }

    m_pView->m_pLineEditSearch->setText(m_pView->m_pLineEditSearch->text()); // reset the isModified flag
    // qDebug() << "isModified =" << m_pView->m_pLineEditSearch->isModified();
  }

}

bool Track::eventFilter(QObject *obj, QEvent *e) {
  if (obj == m_pView->m_pLineEditSearch) {
    // qDebug() << "Track::eventFilter: Received event:" << e->type();
    switch (e->type()) {
      case QEvent::MouseButtonPress:  // Drop entry events which deselect the text
      case QEvent::MouseButtonRelease:
        // qDebug() << "Drop mouse event.";
        return true;
        break;
      case QEvent::FocusIn:
        // qDebug() << "QEvent::FocusIn event intercepted";
        ((QLineEdit *)obj)->selectAll();
        // qDebug() << "hasSelectedText? " << ((QLineEdit *)obj)->hasSelectedText();
        // qDebug() << "So the selected text is now: " << ((QLineEdit *)obj)->selectedText();
        break;
        default: break;
    }
  } else {
    switch (e->type()) {
      case QEvent::Wheel: {
         QWidget* targetWidget;
         targetWidget = ((QWidget *)obj)->childAt(((QWheelEvent *)e)->x(), ((QWheelEvent *)e)->y());
         if (targetWidget == 0)
         {
            //qDebug() << "Mouse Wheel Event -- Null Target Object.";
            return true;
         };
         double wheelDirection = ((QWheelEvent *)e)->delta() / 120.;
         if (qobject_cast<WSliderComposed *>(targetWidget) != 0 || qobject_cast<WKnob *>(targetWidget) != 0) {
           double newValue = qobject_cast<WWidget *>(targetWidget)->getValue() + (wheelDirection);
//           qDebug() << "Mouse Wheel Event -- Destination Object Class Type:"<<targetWidget->metaObject()->className()<<" (target control's value:" << qobject_cast<WWidget *>(targetWidget)->getValue() << " event direction: "<<wheelDirection<<")";
           qobject_cast<WWidget *>(targetWidget)->updateValue(newValue);
//         } else if (qobject_cast<WVisualWaveform *>(targetWidget) != 0) {
//           qDebug() << "Manuiplate a"<<targetWidget->metaObject()->className()<<"...";
//         } else if (qobject_cast<WOverview *>(targetWidget) != 0) {
//           qDebug() << "Manuiplate a"<<targetWidget->metaObject()->className()<<"...";
         }
         return true;
         }
         break;
      // default: qDebug() << "Non SearchEdit Event: " << e->type(); break;
      default: break;
    }
  }
  return false;
}

/** Load and initialize the promo tracks playlist and the special metadata */
void Track::initPromoTracks()
{
    m_qPromoPlaylist.setListName(PLAYLIST_NAME_PROMO);
    if (!checkPromoDirExists())
    {
        //If the promo directory doesn't exist, do nothing.
        return;
    }
    QDir promoDir(m_pConfig->getConfigPath() + QString(MIXXX_PROMO_DIR));

    //Load the extra metadata for these tracks from an XML file and import the promo tracks into the library.
    loadPromoTrackXMLData(promoDir.absolutePath() + "/" + "promo.xml",
                          promoDir.absolutePath());
}

/** Load the special promo track metadata from an XML file. This function also imports these tracks.
 *  @param xmlPath The path to the XML metadata file.
 *  @param promoDirPath The path to the promo tracks directory.
 */
void Track::loadPromoTrackXMLData(QString xmlPath, QString promoDirPath)
{
	//Load settings, and set defaults for anything that we failed to find
	QDomDocument doc("promotracks");
	QFile file(xmlPath);
	if (!file.open(QIODevice::ReadOnly))
	{
		return;
	}
	if (!doc.setContent(&file)) {
		file.close();
		return;
	}
	file.close();

	// print out the element names of all elements that are direct children
	// of the outermost element.
	QDomElement docElem = doc.documentElement();

	QDomNode n = docElem.firstChild();
	while(!n.isNull()) {
		QDomElement e = n.toElement(); // try to convert the node to an element.
		if(!e.isNull()) {
			if (e.tagName() == "track")
			{
				QString filename = promoDirPath + "/" + e.text();
                float bpm = e.attribute("bpm").toFloat();
                QString url = e.attribute("url");
                //TODO: Load comment, but don't overwrite it if the track is already
                //      in the track collection in case the user changed the comment.
                //QString comment =

                //Grab the track from the TrackCollection and add it's metadata...

				TrackInfoObject *newTrack = m_pTrackCollection->getTrack(filename);
                if (newTrack)
                {
                    newTrack->setBpm(bpm);
                    newTrack->setURL(url);
                    //newTrack->setComment(from XML)
					 m_qPromoPlaylist.append(newTrack);
                }
            }

		}
		n = n.nextSibling();
	}

	qDebug() << "Promo playlist has" << m_qPromoPlaylist.getSongNum() << "songs.";

}

bool Track::checkPromoDirExists()
{
    QDir promoDir(m_pConfig->getConfigPath() + QString(MIXXX_PROMO_DIR));
    if (!promoDir.exists())
    {
        qDebug() << "Promo track directory does not exist:" << promoDir.path();
        //Do nothing if there's no promo track directory.
        return false;
    }
    else
        return true;
}


void Track::readXML(QString location)
{
    qDebug() << "Track::readXML" << location;

    // Open XML file
    QFile file(location);
    QDomDocument domXML("Mixxx_Track_List");

    // Check if we can open the file
    if (!file.exists())
    {
        qDebug() << "Track:" << location <<  "does not exist.";
        file.close();
        return;
    }

    // Check if there is a parsing problem
    QString error_msg;
    int error_line;
    int error_column;
    if (!domXML.setContent(&file, &error_msg, &error_line, &error_column))
    {
        qDebug() << "Track: Parse error in" << location;
        qDebug() << "Doctype:" << domXML.doctype().name();
        qDebug() << error_msg << "on line" << error_line << ", column" << error_column;
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
    QString qPlaylistName; //Name of the current playlist
    while (!node.isNull())
    {
        if (node.isElement() && node.nodeName()=="Playlist")
        {
            //Create the playlists internally.
            //If the playlist is "Library" or "Play Queue", insert it into
            //a special spot in the list of playlists.
            qPlaylistName = XmlParse::selectNodeQString(node, "Name");
            if (qPlaylistName == "Library")
            {
                m_qLibraryPlaylist.setTrackCollection(m_pTrackCollection);
                m_qLibraryPlaylist.loadFromXMLNode(node);
            }
            else if (qPlaylistName == "Play Queue")
            {
                m_qPlayqueuePlaylist.setTrackCollection(m_pTrackCollection);
                m_qPlayqueuePlaylist.loadFromXMLNode(node);
            }
            else
                m_qPlaylists.append(new TrackPlaylist(m_pTrackCollection, node));
        }


        node = node.nextSibling();
    }
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

    QListIterator<TrackPlaylist*> it(m_qPlaylists);
    TrackPlaylist* current;
    while (it.hasNext())
    {
        current = it.next();
        progress.setProgress(++i);
        qApp->processEvents();

        QDomElement elementNew = domXML.createElement("Playlist");
        current->writeXML(domXML, elementNew);
        playlistsroot.appendChild(elementNew);

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


void Track::slotScanLibrary()
{
    qDebug() << "Starting Library Scanner...";
    QTime time;
    time.start(); //Time how long the library scan took.

    m_pScanner->scan(m_pConfig->getValueString(ConfigKey("[Playlist]","Directory")));

    //Get the last modified timestamp for the library directory.
    QFileInfo libDir(m_pConfig->getValueString(ConfigKey("[Playlist]","Directory")));
    QString lastModified = libDir.lastModified().toString();
    //... and save that timestamp so we can tell we can know that we need to rescan
    //the library when that timestamp has been changed.
    m_pConfig->set(ConfigKey("[Playlist]","LastModified"), lastModified);

    //Int to UTF-8 string coversion for cmetrics
    QByteArray baElapsed = QString(time.elapsed()).toUtf8();

    #ifdef __C_METRICS__

	cm_writemsg_ascii(MIXXXCMETRICS_LIBRARY_SCAN_TIME,
	                  baElapsed.data());
    #endif
}


void Track::slotDrop(QDropEvent * e)
{
    qDebug() << "track drop";

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

    slotActivatePlaylist(name);
}


/* CTAF: I dont think this function is used actually */
void Track::slotActivatePlaylist(QString name)
{
/*    qDebug() << "playlist change\n";
    // Get pointer to requested playlist
    TrackPlaylist * pNewlist = getPlaylist(name);

    if (pNewlist)
    {
        // Deactivate current playlist
        //if (m_pActivePlaylist)
        //    m_pActivePlaylist->deactivate();
        // Activate new playlist
        m_pActivePlaylist = pNewlist;
        m_pLibraryModel->setTrackPlaylist(m_pActivePlaylist);
        //m_pActivePlaylist->activate(m_pView->m_pTrackTable);
        //m_pActivePlaylist = m_qPlaylists.at(index);
        emit(activePlaylist(pNewlist));
    }*/
}

/** Displays the track listing from a particular playlist in the tracktable view */
void Track::slotShowPlaylist(TrackPlaylist* playlist)
{
    m_pPlaylistModel->setTrackPlaylist(playlist);
    m_pActivePlaylist = playlist;
    m_pView->m_pTrackTableView->setSearchSource(m_pPlaylistModel);

    //We want the same behaviour out of the track table as when it's showing
    //the play queue:
    m_pView->m_pTrackTableView->setTableMode(TABLE_MODE_PLAYQUEUE);
}

TrackPlaylist* Track::getPlaylistByIndex(int index)
{
    return m_qPlaylists.at(index);
}

TrackPlaylistList* Track::getPlaylists()
{
    return &m_qPlaylists;
}

void Track::slotSelectPrevPlaylist(double buttonPressed) {
  if (!buttonPressed) return;
  int index = m_pView->m_pComboBox->currentIndex() - 1;
//  qDebug() << "slotSelectPrevPlaylist index = " << index;
  if (index < 0) return; // Do not allow wrap around -1 -> max
  // if (index < 0) index = m_pView->m_pComboBox->count() - 1;
  m_pView->m_pComboBox->setCurrentIndex(index);
  slotActivatePlaylist(index);
}

void Track::slotSelectNextPlaylist(double buttonPressed) {
  if (!buttonPressed) return;
  int index = m_pView->m_pComboBox->currentIndex() + 1;
//  qDebug() << "slotSelectNextPlaylist index = " << index;
  if (index == m_pView->m_pComboBox->count()) return; // Do not allow wrap around max -> 0
  // if (index == m_pView->m_pComboBox->count()) index = 0;
  m_pView->m_pComboBox->setCurrentIndex(index);
  slotActivatePlaylist(index);
}

void Track::slotActivatePlaylist(int index)
{
    //FIXME: there's gotta be a better signal to use from the combobox
    //       rather than activated(int)... this hardcoded switch is crap

    if ( m_pView->m_pComboBox->itemData(index).canConvert<int>() ){
       index = m_pView->m_pComboBox->itemData(index).value<int>();
    } else { index = TABLE_MODE_LIBRARY; }

    if (m_pView->m_pTrackTableView->getTableMode() == TABLE_MODE_PLAYQUEUE && index != TABLE_MODE_PLAYQUEUE) {
       // Enable sorting mode -- optimize me if you can!
       // TODO: Restore sort column / sort order and restore view position.
       m_pView->m_pTrackTableView->setSortingEnabled(true);
       m_pView->m_pTrackTableView->horizontalHeader()->setSortIndicator(WTrackTableModel::ARTIST, Qt::AscendingOrder);
       m_pView->m_pTrackTableView->m_pSearchFilter->sort(WTrackTableModel::ARTIST);
    } else if (m_pView->m_pTrackTableView->getTableMode() != TABLE_MODE_PLAYQUEUE && index == TABLE_MODE_PLAYQUEUE) {
       // Disable sorting for play queue view, so tracks can be manually ordered using drag and drop...
       // TODO: Save sort  column / sort order from sortIndicator and save view position (see how the search filterbox saves position for example implementation)
       m_pView->m_pTrackTableView->m_pSearchFilter->sort(-1);
       m_pView->m_pTrackTableView->setSortingEnabled(false);
       m_pView->m_pTrackTableView->horizontalHeader()->setSortIndicatorShown(false);
    }

    //Toggled by the ComboBox - This needs to be reorganized...
    switch(index)
    {
      default:
      case TABLE_MODE_LIBRARY: //Library view
        m_pView->m_pTrackTableView->reset();
        m_pView->m_pTrackTableView->setSearchSource(m_pLibraryModel);
        // m_pView->m_pTrackTableView->resizeColumnsToContents();
        resizeColumnsForLibraryMode();
        m_pView->m_pTrackTableView->setTrack(this);
        m_pView->m_pTrackTableView->setTableMode(TABLE_MODE_LIBRARY);
        m_pActivePlaylist = &m_qLibraryPlaylist;
        break;
      case TABLE_MODE_PLAYQUEUE: //Play queue view
        m_pView->m_pTrackTableView->reset();
        m_pView->m_pTrackTableView->setSearchSource(m_pPlayQueueModel);
        m_pView->m_pTrackTableView->resizeColumnsToContents();
        m_pView->m_pTrackTableView->setTrack(this);
        m_pView->m_pTrackTableView->setTableMode(TABLE_MODE_PLAYQUEUE);
        m_pActivePlaylist = &m_qPlayqueuePlaylist;
        break;
      case TABLE_MODE_BROWSE: //Browse mode view
        m_pView->m_pTrackTableView->reset();
        m_pView->m_pTrackTableView->setDirModel();
        m_pView->m_pTrackTableView->resizeColumnsToContents();
        m_pView->m_pTrackTableView->setTrack(this);
        m_pView->m_pTrackTableView->setTableMode(TABLE_MODE_BROWSE);
        break;
      case TABLE_MODE_PLAYLISTS: //Playlist List Model
        m_pView->m_pTrackTableView->reset();
        m_pView->m_pTrackTableView->setPlaylistListModel(m_pPlaylistListModel);
        //m_pView->m_pTrackTableView->setSearchSource(m_pPlaylistListModel); //Doesn't work right yet
        m_pView->m_pTrackTableView->resizeColumnsToContents();
        m_pView->m_pTrackTableView->setTrack(this);
        m_pView->m_pTrackTableView->setTableMode(TABLE_MODE_PLAYLISTS);
        //FIXME ... return here or something? - Albert
        break;
      case TABLE_MODE_PROMO: //Promo Tracks
        m_pView->m_pTrackTableView->reset();
        m_pView->m_pTrackTableView->setSearchSource(m_pPromoModel);
        m_pView->m_pTrackTableView->resizeColumnsToContents();
        m_pView->m_pTrackTableView->setTrack(this);
        m_pView->m_pTrackTableView->setTableMode(TABLE_MODE_PROMO);
        m_pActivePlaylist = &m_qPromoPlaylist;
        break;
#ifdef __IPOD__
      case TABLE_MODE_IPOD: // Ipod
        m_pView->m_pTrackTableView->reset();
        m_pView->m_pTrackTableView->setSearchSource(m_pIPodModel);
        m_pView->m_pTrackTableView->resizeColumnsToContents();
        m_pView->m_pTrackTableView->setTrack(this);
        m_pView->m_pTrackTableView->setTableMode(TABLE_MODE_IPOD);
        m_pActivePlaylist = &m_qIPodPlaylist;
        break;
#endif
    }
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

    // Update anything that views the playlists
    updatePlaylistViews();
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
#if QT_VERSION >= 0x040400
        m_qPlaylists.removeOne(list);
#else
	m_qPlaylists.removeAt(m_qPlaylists.indexOf(list));
#endif
        delete list;
    }

    if (bActivateOtherList)
    {
        if (m_qPlaylists.count()==0)
            slotNewPlaylist();

        slotActivatePlaylist(m_qLibraryPlaylist.getListName());
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

    if (pTempPlaylist != NULL)
    {
        m_qPlaylists.append(pTempPlaylist);
    }
    updatePlaylistViews();
}

TrackPlaylist * Track::getPlaylist(QString qName)
{
    QListIterator<TrackPlaylist*> it(m_qPlaylists);
    TrackPlaylist* current;
    while (it.hasNext())
    {
        current = it.next();
        if (current->getListName()==qName)
            return current;
    }
    return 0;
}

/** Sends a playlist to the play queue */
void Track::slotSendToPlayqueue(TrackPlaylist* playlist)
{
    for (int i = 0; i < playlist->getSongNum(); i++)
    {
        m_qPlayqueuePlaylist.addTrack(playlist->at(i));
    }
}

void Track::slotSendToPlayqueue(TrackInfoObject * pTrackInfoObject)
{
    m_qPlayqueuePlaylist.addTrack(pTrackInfoObject);
}


void Track::slotSendToPlayqueue(QString filename)
{
    TrackInfoObject* pTrack = m_pTrackCollection->getTrack(filename);
    if (pTrack)
        m_qPlayqueuePlaylist.addTrack(pTrack);
}

void Track::slotSendToPlaylist(TrackPlaylist* playlist, TrackInfoObject* pTrackInfoObject)
{
    playlist->addTrack(pTrackInfoObject);
}

void Track::slotSendToPlaylist(TrackPlaylist* playlist, QString filename)
{
    TrackInfoObject* pTrack = m_pTrackCollection->getTrack(filename);
    if (pTrack)
        playlist->addTrack(pTrack);
}

void Track::slotLoadPlayer1(TrackInfoObject * pTrackInfoObject, bool bStartFromEndPos)
{

    QString filename = pTrackInfoObject->getLocation();
    qDebug() << "Load to player1:" << filename;
    if (!filename.isEmpty())
    {
        // Check if filename is valid
        QFileInfo finfo(filename);
        if (!finfo.exists())
        {
            qDebug() << "Song in library not found on at" << filename << "on disk.";
            qDebug() << "Removing from library...";
            m_pTrackCollection->removeTrack(pTrackInfoObject); //Remove the track from the library.
            m_pView->m_pTrackTableView->repaintEverything(); //Refresh the library view.
            return;
        }
    }
    else return; // empty filename

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
    m_pBuffer1->loadTrack(m_pTrackPlayer1);
    // TODO(rryan) : We no longer will start at the end position, fix that.

    //The rest of the track loading code gets executed in slotFinishLoadingPlayer1(), 
    //which gets called when Reader emits a signal saying it's done loading the song. This prevents
    //several race conditions.
}

/** This slot gets called when Reader has finished loading a new track, because Reader loads it in a separate thread.
    This code is essentially the second half of slotLoadPlayer1(), which is the stuff that needs to be executed
    after the song has been loaded into the reader. Before this function was implemented, there were a handful
    of race conditions in slotLoadPlayer1() where stuff was executed at the same time as Reader was loading the 
    track. This function explicitly solves that problem. */
void Track::slotFinishLoadingPlayer1(TrackInfoObject* pTrackInfoObject)
{
    bool bStartFromEndPos = false;
    // Read the tags if required
    if(!m_pTrackPlayer1->getHeaderParsed())
        SoundSourceProxy::ParseHeader(m_pTrackPlayer1);

    // Set waveform summary display
    m_pTrackPlayer1->setOverviewWidget(m_pView->m_pOverviewCh1);
    
    // This has to happen before the AnalyserQueue works on the track.
    m_pTrackPlayer1->setVisualResampleRate(m_pVisualResampleCh1->get());

    // Queue the track for BPM/Waveform/Wavesummary/etc. analysis
	m_analyserQueue->queueAnalyseTrack(pTrackInfoObject);

    // Set Engine file BPM and duration ControlObjects
    m_pTrackPlayer1->setBpmControlObject(ControlObject::getControl(ConfigKey("[Channel1]","file_bpm")));
    m_pTrackPlayer1->setDurationControlObject(ControlObject::getControl(ConfigKey("[Channel1]","duration")));

    // Update TrackInfoObject of the helper class
    PlayerInfo::Instance().setTrackInfo(1, m_pTrackPlayer1);
    
    //Set the cue point, if it was saved.
    m_pCuePointCh1->slotSet(m_pTrackPlayer1->getCuePoint());

    //Seek to cue position if we're not starting at end of song and cue recall is on
    if (!bStartFromEndPos) {
        int cueRecall = m_pConfig->getValueString(ConfigKey("[Controls]","CueRecall")).toInt();
        if (cueRecall == 0) { //If cue recall is ON in the prefs, then we're supposed to seek to the cue point on song load. 
            //Note that cueRecall == 0 corresponds to "ON", not OFF.
            float cue_point = m_pTrackPlayer1->getCuePoint();
            m_pBuffer1->slotControlSeekAbs(cue_point);
        }
    }

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
    QString filename = pTrackInfoObject->getLocation();
    qDebug() << "Load to player2:" << filename;
    if (!filename.isEmpty())
    {
        // Check if filename is valid
        QFileInfo finfo(filename);
        if (!finfo.exists())
        {
            qDebug() << "Song in library not found on at" << filename << "on disk.";
            qDebug() << "Removing from library...";
            m_pTrackCollection->removeTrack(pTrackInfoObject); //Remove the track from the library.
            m_pView->m_pTrackTableView->repaintEverything(); //Refresh the library view.
            return;
        }
    }
    else return; // empty filename

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
    m_pBuffer2->loadTrack(m_pTrackPlayer2);
    // TODO(rryan) : We no longer will start at the end position, fix that.
}

/** See comment for slotFinishLoadingPlayer1 */
void Track::slotFinishLoadingPlayer2(TrackInfoObject* pTrackInfoObject)
{
    bool bStartFromEndPos = false;
    // Read the tags if required
    if(!m_pTrackPlayer2->getHeaderParsed())
        SoundSourceProxy::ParseHeader(m_pTrackPlayer2);

    // Set waveform summary display
    m_pTrackPlayer2->setOverviewWidget(m_pView->m_pOverviewCh2);
    
    // This has to happen before the AnalyserQueue works on the track.
    m_pTrackPlayer2->setVisualResampleRate(m_pVisualResampleCh2->get());

    // Queue the track for BPM/Waveform/Wavesummary/etc. analysis
    m_analyserQueue->queueAnalyseTrack(pTrackInfoObject);

    // Set Engine file BPM ControlObject
    m_pTrackPlayer2->setBpmControlObject(ControlObject::getControl(ConfigKey("[Channel2]","file_bpm")));
    m_pTrackPlayer2->setDurationControlObject(ControlObject::getControl(ConfigKey("[Channel2]","duration")));

    // Update TrackInfoObject of the helper class
    PlayerInfo::Instance().setTrackInfo(2, m_pTrackPlayer2);
    
    //Set the cue point, if it was saved.
    m_pCuePointCh2->slotSet(m_pTrackPlayer2->getCuePoint());
    
    //Seek to cue position if we're not starting at end of song and cue recall is on
    if (!bStartFromEndPos) {
        int cueRecall = m_pConfig->getValueString(ConfigKey("[Controls]","CueRecall")).toInt();
        if (cueRecall == 0) { //If cue recall is ON in the prefs, then we're supposed to seek to the cue point on song load. 
            //Note that cueRecall == 0 corresponds to "ON", not OFF.
            float cue_point = m_pTrackPlayer2->getCuePoint();
            m_pBuffer2->slotControlSeekAbs(cue_point);
        }
    }


    // Set duration in playpos widget
//    if (m_pView->m_pNumberPosCh2)
//        m_pView->m_pNumberPosCh2->setDuration(m_pTrackPlayer2->getDuration());

    // Write info to text display
    if (m_pView->m_pTextCh2)
        m_pView->m_pTextCh2->setText(m_pTrackPlayer2->getInfo());

    emit(newTrackPlayer2(m_pTrackPlayer2));

}


void Track::slotLoadPlayer1(QString filename, bool bStartFromEndPos)
{
    TrackInfoObject * pTrack = m_pTrackCollection->getTrack(filename);
    if (pTrack)
        slotLoadPlayer1(pTrack, bStartFromEndPos);
}

void Track::slotLoadPlayer2(QString filename, bool bStartFromEndPos)
{
    TrackInfoObject * pTrack = m_pTrackCollection->getTrack(filename);
    if (pTrack)
        slotLoadPlayer2(pTrack, bStartFromEndPos);
}

TrackPlaylist * Track::getActivePlaylist()
{
    return m_pActivePlaylist;
}

void Track::slotEndOfTrackPlayer1(double val)
{
//    qDebug() << "end of track " << val;

    if (val==0.)
        return;

    switch ((int)m_pEndOfTrackModeCh1->get())
    {
    case TRACK_END_MODE_NEXT:
        if (m_pTrackPlayer1)
        {
            TrackInfoObject *pTrack = NULL;
            bool bStartFromEndPos = false;

            //Load the next song...
            if (m_pPlayPositionCh1->get()>0.5)
            {
               //If the play queue has another song in it, load that...
               int idx = m_qPlayqueuePlaylist.indexOf(m_pTrackPlayer1);
               if (idx + 1 < m_qPlayqueuePlaylist.count())
               {
                   pTrack = m_qPlayqueuePlaylist.at(idx + 1);
               }
               else
               {
                   //Stop playback at the end of the play queue.
                   m_pPlayButtonCh1->slotSet(0.);
               }
            }
            else //Load previous track
            {
                //If the play queue has a previous song in it, load that...
                int idx = m_qPlayqueuePlaylist.indexOf(m_pTrackPlayer1);
                if (idx - 1 >= 0)
                {
                    pTrack = m_qPlayqueuePlaylist.at(idx - 1);
                }

                bStartFromEndPos = true;
            }

            if (bStartFromEndPos)
                qDebug() << "start from end";

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
            TrackInfoObject *pTrack = NULL;
            bool bStartFromEndPos = false;

            //Load the next song...
            if (m_pPlayPositionCh2->get()>0.5)
            {
               //If the play queue has another song in it, load that...
               int idx = m_qPlayqueuePlaylist.indexOf(m_pTrackPlayer2);
               if (idx + 1 < m_qPlayqueuePlaylist.count())
               {
                   pTrack = m_qPlayqueuePlaylist.at(idx + 1);
               }
               else
               {
                   //Stop playback at the end of the play queue.
                   m_pPlayButtonCh2->slotSet(0.);
               }
            }
            else //Load previous track
            {
                //If the play queue has a previous song in it, load that...
                int idx = m_qPlayqueuePlaylist.indexOf(m_pTrackPlayer2);
                if (idx - 1 >= 0)
                {
                    pTrack = m_qPlayqueuePlaylist.at(idx - 1);
                }

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

void Track::slotLoadSelectedTrackCh1(double v)
{
    QModelIndex index;
    TrackInfoObject *pTrack;
    // Only load on key presses and if we're not in browse mode
    if (v && m_pView->m_pTrackTableView->m_pTable) {
        // Fetch the currently selected track
        index = m_pView->m_pTrackTableView->m_pSearchFilter->mapToSource(m_pView->m_pTrackTableView->currentIndex());
        int row = index.row();
        if (row < 0 || row >= m_pView->m_pTrackTableView->m_pTable->m_pTrackPlaylist->size()) return; // prevent QList out-of-bounds asserts
        pTrack = m_pView->m_pTrackTableView->m_pTable->m_pTrackPlaylist->at(index.row());
        // If there is one, load it
	if (pTrack) slotLoadPlayer1(pTrack);
    }
}

void Track::slotLoadSelectedTrackCh2(double v)
{
    QModelIndex index;
    TrackInfoObject *pTrack;
    // Only load on key presses and if we're not in browse mode
    if (v && m_pView->m_pTrackTableView->m_pTable) {
        // Fetch the currently selected track
        index = m_pView->m_pTrackTableView->m_pSearchFilter->mapToSource(m_pView->m_pTrackTableView->currentIndex());
        pTrack = m_pView->m_pTrackTableView->m_pTable->m_pTrackPlaylist->at(index.row());
	    // If there is one, load it
        if (pTrack) slotLoadPlayer2(pTrack);
    }
}

void Track::slotLoadSelectedIntoFirstStopped(double v)
{
    if (v)
    {
        if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()!=1.)
            this->slotLoadSelectedTrackCh1(v);
        else if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()!=1.)
            this->slotLoadSelectedTrackCh2(v);
    }
}

void Track::slotSelectNextTrack(double v)
{
    // Only move on key presses
    if (v) m_pView->m_pTrackTableView->selectNext();
}

void Track::slotSelectPrevTrack(double v)
{
    // Only move on key presses
    if (v) m_pView->m_pTrackTableView->selectPrevious();
}

void Track::slotSelectTrackKnob(double v)
{
    int i = (int)v;
    m_pView->m_pTrackTableView->setUpdatesEnabled(false);
    while(i != 0)
    {
        if(i > 0)
        {
            m_pView->m_pTrackTableView->selectNext();
            i--;
        }
        else
        {
            m_pView->m_pTrackTableView->selectPrevious();
            i++;
        }
    }
    m_pView->m_pTrackTableView->setUpdatesEnabled(true);
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

void Track::updatePlaylistViews()
   {
    // Sort list
    qSort(m_qPlaylists);

    //Tell the track table view to update it's right-click/send-to-playlist menu.
    m_pView->m_pTrackTableView->updatePlaylistActions();

    qDebug() << "FIXME: Need to tell the m_pPlaylistListModel to refresh in" << __FILE__ << "on line:" << __LINE__;
    //if (m_pView && m_pView->m_pTrackTableView)
    //    m_pView->m_pTrackTableView->repaintEverything(); //Crashy crashy at startup for some reason :(

    // Update tree view
    //if (m_pView->m_pTreeView)
    //    m_pView->m_pTreeView->updatePlaylists(&m_qPlaylists);

    //m_pPlayQueueModel->setTrackPlaylist(m_qPlaylists)

    // Update menu
    //emit(updateMenu(&m_qPlaylists));

    // Set active
    if (m_pActivePlaylist)
        emit(activePlaylist(m_pActivePlaylist));
   }

/** Checks if the library directory's "last modified" timestamp has been changed */
/* @return true if the library has been modified since it was last rescanned
   @return false if the library has not been modified since it was last rescanned
*/
bool Track::checkLibraryLastModified()
{
    //Get some info about the library directory
    QFileInfo libDir(m_pConfig->getValueString(ConfigKey("[Playlist]","Directory")));

    //Get the last modified timestamp for the library directory.
    QString lastModified = libDir.lastModified().toString();

    //Compare the timestamp with our own stored timestamp, to see if the library
    //has been modified since we last scanned it.
    if (lastModified == m_pConfig->getValueString(ConfigKey("[Playlist]","LastModified")))
    {
        return false;
    }
    else
    {
        //Note: The LastModified config key gets updated in slotScanLibrary(), for consistency.

        return true;
    }
}

/** Runs the BPM detection on every track in the TrackCollection that doesn't already have a BPM. */
void Track::slotBatchBPMDetection()
{
    /* Possible use cases:
       1) BPM detect whole library
       2) BPM detect play queue or a specific play list
    */

    // TODO: Abort batch processing if either deck is playing and tell user...
    int processed = 0;
    for (int i =0; i < m_pTrackCollection->getSize(); i++) {
      TrackInfoObject* cur_track = m_pTrackCollection->getTrack(i);

      if (!cur_track->isValid()) continue; // Skip invalid track objects

      // Batch Process Wave Summary
      if (cur_track->getWaveSummary() == NULL || cur_track->getWaveSummary()->size() == 0) {
        processed++;
        // TODO: convert the qDebug statement to a status dialog box.
        qDebug() << "Track #"<< i << "= Batch job #"<< processed << ":" << cur_track->getTitle() << "by" << cur_track->getArtist() << "has WaveSummary: false -- has BPM value:" << (cur_track->getBpm() > 0);

        // TODO(rryan) make an AnalyserQueue here
        // Batch Process BPM
        //if (cur_track->getBpm() == 0) {
        //cur_track->sendToBpmQueue();
        //}
      }

      // FIXME: remove the song count limitation when BPM detect doesn't crash...
      if (processed == 100) { qDebug() << "----- BPM detection/Waveform generation batch processing limit of " << processed << " songs has been reached. Rerun to detect more songs."; break; }
    }
    // Save the track database to disk...
    writeXML(m_pConfig->getValueString(ConfigKey("[Playlist]","Listfile")));
}

