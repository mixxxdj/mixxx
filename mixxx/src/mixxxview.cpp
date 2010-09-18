/***************************************************************************
                          mixxxview.cpp  -  description
                             -------------------
    begin                : Mon Feb 18 09:48:17 CET 2002
    copyright            : (C) 2002 by Tue and Ken .Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include <QtGui>

#include "mixxxview.h"
#include "widget/wwidget.h"
#include "widget/wabstractcontrol.h"
#include "widget/wknob.h"
#include "widget/wpushbutton.h"
#include "widget/wslider.h"
#include "widget/wslidercomposed.h"
#include "widget/wdisplay.h"
#include "widget/wvumeter.h"
#include "widget/wstatuslight.h"
#include "widget/wlabel.h"
#include "widget/wnumber.h"
#include "widget/wnumberpos.h"
#include "widget/wnumberbpm.h"
#include "widget/wnumberrate.h"
#include "widget/wpixmapstore.h"
#include "widget/wsearchlineedit.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrary.h"


#include "widget/woverview.h"
#include "mixxxkeyboard.h"
#include "controlobject.h"
#include "controlobjectthreadwidget.h"
#include "waveformviewerfactory.h"
#include "waveform/waveformrenderer.h"
#include "widget/wvisualsimple.h"
#include "widget/wglwaveformviewer.h"
#include "widget/wwaveformviewer.h"
#include "trackinfoobject.h"
#include "player.h"


#include "imgloader.h"
#include "imginvert.h"
#include "imgcolor.h"
#include "widget/wskincolor.h"
#include "mixxx.h"
#ifdef __LADSPA__
#include "ladspaview.h"
#endif
#include "defs_promo.h"
#include "library/library.h"
#include "library/trackcollection.h"
#include "library/librarytablemodel.h"
#include "library/rhythmboxtrackmodel.h"
#include "library/rhythmboxplaylistmodel.h"
#include "library/browsetablemodel.h"

MixxxView::MixxxView(QWidget* parent, ConfigObject<ConfigValueKbd>* kbdconfig,
		     QString qSkinPath, ConfigObject<ConfigValue>* pConfig,
		     Player* player1, Player* player2,
		     Library* pLibrary) : QWidget(parent) {
    view = 0;
    m_pconfig = pConfig;
    m_pPlayer1 = player1;
    m_pPlayer2 = player2;
    m_pLibrary = pLibrary;

    m_pKeyboard = new MixxxKeyboard(kbdconfig);
    installEventFilter(m_pKeyboard);

    // Try to open the file pointed to by qSkinPath
    QDomElement docElem = openSkin(qSkinPath);

#ifdef __WINDOWS__
#ifndef QT3_SUPPORT
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::MemoryOptim);
#endif
#endif

    m_pWaveformRendererCh1 = new WaveformRenderer("[Channel1]");
    m_pWaveformRendererCh2 = new WaveformRenderer("[Channel2]");

    connect(m_pPlayer1, SIGNAL(unloadingTrack(TrackPointer)),
            m_pWaveformRendererCh1, SLOT(slotUnloadTrack(TrackPointer)));
    connect(m_pPlayer2, SIGNAL(unloadingTrack(TrackPointer)),
            m_pWaveformRendererCh2, SLOT(slotUnloadTrack(TrackPointer)));

    // Default values for visuals
    m_pTextCh1 = 0;
    m_pTextCh2 = 0;
    m_pVisualCh1 = 0;
    m_pVisualCh2 = 0;
    m_pNumberPosCh1 = 0;
    m_pNumberPosCh2 = 0;
    m_pNumberBpmCh1 = 0;
    m_pNumberBpmCh2 = 0;
    m_pSliderRateCh1 = 0;
    m_pSliderRateCh2 = 0;
    m_bVisualWaveform = false;
    m_pOverviewCh1 = 0;
    m_pOverviewCh2 = 0;
    m_pLineEditSearch = 0;
    m_pTabWidget = 0;
    m_pTabWidgetLibraryPage = 0;
#ifdef __LADSPA__
    m_pTabWidgetEffectsPage = 0;
#endif
    m_pLibraryPageLayout = new QGridLayout();
    m_pEffectsPageLayout = new QGridLayout();
    m_pSplitter = 0;
    m_pLibrarySidebar = 0;
    m_pLibrarySidebarPage = 0; //The sidebar and search widgets get embedded in this.

    // Default to showing absolute durations instead of duration remaining. The
    // preferences will change this to what the user prefers.
    m_bDurationRemain = false;

    m_textCh1 = "";
    m_textCh2 = "";

    setupColorScheme(docElem, pConfig);

    // Load all widgets defined in the XML file
    createAllWidgets(docElem, parent, pConfig);

#ifdef __WINDOWS__
#ifndef QT3_SUPPORT
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::NormalOptim);
#endif
#endif

 	 //Connect the players to the waveform overview widgets so they
 	 //update when a new track is loaded.
 	connect(m_pPlayer1, SIGNAL(newTrackLoaded(TrackPointer)),
          m_pOverviewCh1, SLOT(slotLoadNewWaveform(TrackPointer)));
  connect(m_pPlayer1, SIGNAL(unloadingTrack(TrackPointer)),
          m_pOverviewCh1, SLOT(slotUnloadTrack(TrackPointer)));
	connect(m_pPlayer2, SIGNAL(newTrackLoaded(TrackPointer)),
          m_pOverviewCh2, SLOT(slotLoadNewWaveform(TrackPointer)));
  connect(m_pPlayer2, SIGNAL(unloadingTrack(TrackPointer)),
          m_pOverviewCh2, SLOT(slotUnloadTrack(TrackPointer)));

	//Connect the players to some other widgets, so they get updated when a
	//new track is loaded.
	connect(m_pPlayer1, SIGNAL(newTrackLoaded(TrackPointer)),
          this, SLOT(slotUpdateTrackTextCh1(TrackPointer)));
  connect(m_pPlayer1, SIGNAL(unloadingTrack(TrackPointer)),
          this, SLOT(slotClearTrackTextCh1(TrackPointer)));
	connect(m_pPlayer2, SIGNAL(newTrackLoaded(TrackPointer)),
          this, SLOT(slotUpdateTrackTextCh2(TrackPointer)));
  connect(m_pPlayer2, SIGNAL(unloadingTrack(TrackPointer)),
          this, SLOT(slotClearTrackTextCh2(TrackPointer)));

	//Setup a connection that allows us to connect the TrackInfoObjects that
	//get loaded into the players to the waveform overview widgets. We don't
	//want the TrackInfoObjects talking directly to the GUI code, so this is
	//a way for us to keep this modular. (The TrackInfoObjects need to
	//notify the waveform overview widgets to update once the waveform
	//summary has finished generating. This connection gives us a way to
	//create that connection at runtime.)
	connect(m_pPlayer1, SIGNAL(newTrackLoaded(TrackPointer)),
		this, SLOT(slotSetupTrackConnectionsCh1(TrackPointer)));
	connect(m_pPlayer2, SIGNAL(newTrackLoaded(TrackPointer)),
		this, SLOT(slotSetupTrackConnectionsCh2(TrackPointer)));

	// Connect search box signals to the library
	connect(m_pLineEditSearch, SIGNAL(search(const QString&)),
          m_pLibraryWidget, SLOT(search(const QString&)));
	connect(m_pLineEditSearch, SIGNAL(searchCleared()),
          m_pLibraryWidget, SLOT(searchCleared()));
	connect(m_pLineEditSearch, SIGNAL(searchStarting()),
          m_pLibraryWidget, SLOT(searchStarting()));
  connect(m_pLibrary, SIGNAL(restoreSearch(const QString&)),
          m_pLineEditSearch, SLOT(restoreSearch(const QString&)));

  int desired_fps = 40;
  float update_interval = 1000.0f / desired_fps;
  m_guiTimer.start(update_interval);
}

MixxxView::~MixxxView()
{
    //m_qWidgetList.clear();

    if(m_pVisualCh1) {
	m_qWidgetList.remove(m_pVisualCh1);
	delete m_pVisualCh1;
	m_pVisualCh1 = NULL;
    }

    if(m_pVisualCh2) {
	m_qWidgetList.remove(m_pVisualCh2);
	delete m_pVisualCh2;
	m_pVisualCh2 = NULL;
    }

    if(m_pWaveformRendererCh1) {
	delete m_pWaveformRendererCh1;
	m_pWaveformRendererCh1 = NULL;
    }

    if(m_pWaveformRendererCh2) {
	delete m_pWaveformRendererCh2;
	m_pWaveformRendererCh2 = NULL;
    }
}

void MixxxView::checkDirectRendering()
{
    // IF
    //  * A waveform viewer exists
    // AND
    //  * The waveform viewer is an OpenGL waveform viewer
    // AND
    //  * The waveform viewer does not have direct rendering enabled.
    // THEN
    //  * Warn user

    // TODO rryan -- re-integrate with 'new' GL viewer


    if((m_pVisualCh1 &&
	WaveformViewerFactory::getWaveformViewerType(m_pVisualCh1) == WAVEFORM_GL &&
	!((WGLWaveformViewer *)m_pVisualCh1)->directRendering()) ||
       (m_pVisualCh2 &&
	WaveformViewerFactory::getWaveformViewerType(m_pVisualCh2) == WAVEFORM_GL &&
	!((WGLWaveformViewer *)m_pVisualCh2)->directRendering()))
        {
	    if(m_pconfig->getValueString(ConfigKey("[Direct Rendering]", "Warned")) != QString("yes"))
		{
		    QMessageBox::warning(0, "OpenGL Direct Rendering",
					 "Direct rendering is not enabled on your machine.\n\nThis means that the waveform displays will be very\nslow and take a lot of CPU time. Either update your\nconfiguration to enable direct rendering, or disable\nthe waveform displays in the control panel by\nselecting \"Simple\" under waveform displays.\nNOTE: In case you run on NVidia hardware,\ndirect rendering may not be present, but you will\nnot experience a degradation in performance.");
		    m_pconfig->set(ConfigKey("[Direct Rendering]", "Warned"), ConfigValue(QString("yes")));
		}
	}

}

bool MixxxView::activeWaveform()
{
    return m_bVisualWaveform;
}

bool MixxxView::compareConfigKeys(QDomNode node, QString key)
{
    QDomNode n = node;

    // Loop over each <Connection>, check if it's ConfigKey matches key
    while (!n.isNull())
    {
        n = WWidget::selectNode(n, "Connection");
        if (!n.isNull())
        {
            if  (WWidget::selectNodeQString(n, "ConfigKey").contains(key))
                return true;
        }
    }
    return false;
}

ImgSource * MixxxView::parseFilters(QDomNode filt) {

    // TODO: Move this code into ImgSource
    if (!filt.hasChildNodes()) {
        return 0;
    }

    ImgSource * ret = new ImgLoader();

    QDomNode f = filt.firstChild();

    while (!f.isNull()) {
        QString name = f.nodeName().toLower();
        if (name == "invert") {
            ret = new ImgInvert(ret);
        } else if (name == "hueinv") {
            ret = new ImgHueInv(ret);
        } else if (name == "add") {
            ret = new ImgAdd(ret, WWidget::selectNodeInt(f, "Amount"));
        } else if (name == "scalewhite") {
            ret = new ImgScaleWhite(ret, WWidget::selectNodeFloat(f, "Amount"));
        } else if (name == "hsvtweak") {
            int hmin = 0;
            int hmax = 255;
            int smin = 0;
            int smax = 255;
            int vmin = 0;
            int vmax = 255;
            float hfact = 1.0f;
            float sfact = 1.0f;
            float vfact = 1.0f;
            int hconst = 0;
            int sconst = 0;
            int vconst = 0;

            if (!f.namedItem("HMin").isNull()) { hmin = WWidget::selectNodeInt(f, "HMin"); }
            if (!f.namedItem("HMax").isNull()) { hmax = WWidget::selectNodeInt(f, "HMax"); }
            if (!f.namedItem("SMin").isNull()) { smin = WWidget::selectNodeInt(f, "SMin"); }
            if (!f.namedItem("SMax").isNull()) { smax = WWidget::selectNodeInt(f, "SMax"); }
            if (!f.namedItem("VMin").isNull()) { vmin = WWidget::selectNodeInt(f, "VMin"); }
            if (!f.namedItem("VMax").isNull()) { vmax = WWidget::selectNodeInt(f, "VMax"); }

            if (!f.namedItem("HConst").isNull()) { hconst = WWidget::selectNodeInt(f, "HConst"); }
            if (!f.namedItem("SConst").isNull()) { sconst = WWidget::selectNodeInt(f, "SConst"); }
            if (!f.namedItem("VConst").isNull()) { vconst = WWidget::selectNodeInt(f, "VConst"); }

            if (!f.namedItem("HFact").isNull()) { hfact = WWidget::selectNodeFloat(f, "HFact"); }
            if (!f.namedItem("SFact").isNull()) { sfact = WWidget::selectNodeFloat(f, "SFact"); }
            if (!f.namedItem("VFact").isNull()) { vfact = WWidget::selectNodeFloat(f, "VFact"); }

            ret = new ImgHSVTweak(ret, hmin, hmax, smin, smax, vmin, vmax, hfact, hconst,
                                  sfact, sconst, vfact, vconst);
        } else {
            qDebug() << "Unkown image filter:" << name;
        }
        f = f.nextSibling();
    }

    return ret;
}

QDomElement MixxxView::openSkin(QString qSkinPath) {

    // Path to image files
    WWidget::setPixmapPath(qSkinPath.append("/"));

    // Read XML file
    QDomDocument skin("skin");
    QFile file(WWidget::getPath("skin.xml"));
    QFileInfo fi(file);
    QByteArray qbaFilename = fi.fileName().toUtf8();

    if (!file.open(QIODevice::ReadOnly))
    {
        qFatal("Could not open skin definition file: %s", qbaFilename.constData());
    }
    if (!skin.setContent(&file))
    {
        qFatal("Error parsing skin definition file: %s", qbaFilename.constData());
    }

    file.close();
    return skin.documentElement();
}

void MixxxView::setupColorScheme(QDomElement docElem, ConfigObject<ConfigValue> * pConfig) {

    QDomNode colsch = docElem.namedItem("Schemes");
    if (!colsch.isNull() && colsch.isElement()) {
        QString schname = pConfig->getValueString(ConfigKey("[Config]","Scheme"));
        QDomNode sch = colsch.firstChild();

        bool found = false;

		if (schname.isEmpty()) {
			// If no scheme stored, accept the first one in the file
			found = true;
		}

        while (!sch.isNull() && !found) {
            QString thisname = WWidget::selectNodeQString(sch, "Name");
            if (thisname == schname) {
                found = true;
            } else {
                sch = sch.nextSibling();
            }
        }

        if (found) {
            ImgSource * imsrc = parseFilters(sch.namedItem("Filters"));
            WPixmapStore::setLoader(imsrc);
            WSkinColor::setLoader(imsrc);
        } else {
            WPixmapStore::setLoader(0);
            WSkinColor::setLoader(0);
        }
    } else {
        WPixmapStore::setLoader(0);
        WSkinColor::setLoader(0);
    }
}

void MixxxView::createAllWidgets(QDomElement docElem,
                                 QWidget * parent,
                                 ConfigObject<ConfigValue> * pConfig) {
    WAbstractControl *currentControl = 0;
    QDomNode node = docElem.firstChild();
    while (!node.isNull())
    {
        currentControl = 0;
        if (node.isElement())
        {
          /*############## NOT PERSISTENT OBJECT ##############*/
            //printf("%s\n", node.nodeName());
            if (node.nodeName()=="PushButton")
            {
                WPushButton * p = new WPushButton(this);
                p->setup(node);
                p->installEventFilter(m_pKeyboard);
                m_qWidgetList.append(p);
            }
            else if (node.nodeName()=="Knob")
            {
                WKnob * p = new WKnob(this);
                p->setup(node);
                p->installEventFilter(m_pKeyboard);
                m_qWidgetList.append(p);
                currentControl = qobject_cast<WAbstractControl*>(p);
            }
            else if (node.nodeName()=="Label")
            {
                WLabel * p = new WLabel(this);
                p->setup(node);

                m_qWidgetList.append(p);
            }
            else if (node.nodeName()=="Number")
            {
                WNumber * p = new WNumber(this);
                p->setup(node);
                p->installEventFilter(m_pKeyboard);
                m_qWidgetList.append(p);
            }
            else if (node.nodeName()=="NumberBpm")
            {
                if (m_pNumberBpmCh1 != NULL || m_pNumberBpmCh2) {
                    qDebug() << "WARNING: BPM widget pointers wre not reset on GUI reboot.";
                }
                // NOTE: The BPM widgets are added to the widget list (so they
                // will be auto-deleted) and also assigned to m_pNumberBpmCh1/2.
                if (WWidget::selectNodeInt(node, "Channel")==1)
                {
                    WNumberBpm * p = new WNumberBpm("[Channel1]", this);
                    p->setup(node);
                    p->installEventFilter(m_pKeyboard);
                    m_qWidgetList.append(p);
                    m_pNumberBpmCh1 = p; //100% of the source code in this file sucks.
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                {
                    WNumberBpm * p = new WNumberBpm("[Channel2]", this);
                    p->setup(node);
                    p->installEventFilter(m_pKeyboard);
                    m_qWidgetList.append(p);
                    m_pNumberBpmCh2 = p;
                }
            }
            else if (node.nodeName()=="NumberRate")
            {
                QColor c(255,255,255);
                if (!WWidget::selectNode(node, "BgColor").isNull()) {
                    c.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
                }

                QPalette palette;
                //palette.setBrush(QPalette::Background, WSkinColor::getCorrectColor(c));
                palette.setBrush(QPalette::Button, Qt::NoBrush);

                if (WWidget::selectNodeInt(node, "Channel")==1)
                {
                    WNumberRate * p = new WNumberRate("[Channel1]", this);
                    p->setup(node);
                    p->installEventFilter(m_pKeyboard);
                    m_qWidgetList.append(p);
                    p->setPalette(palette);
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                {
                    WNumberRate * p = new WNumberRate("[Channel2]", this);
                    p->setup(node);
                    p->installEventFilter(m_pKeyboard);
                    m_qWidgetList.append(p);
                    p->setPalette(palette);
                }
            }
            else if (node.nodeName()=="Display")
            {
                WDisplay * p = new WDisplay(this);
                p->setup(node);
                p->installEventFilter(m_pKeyboard);
                m_qWidgetList.append(p);
            }
            else if (node.nodeName()=="Background")
            {
                QString filename = WWidget::selectNodeQString(node, "Path");
                QPixmap *background = WPixmapStore::getPixmapNoCache(WWidget::getPath(filename));
                QColor c(0,0,0); // Default background color is now black, if people want to do <invert/> filters they'll have to figure something out for this.
                QLabel *bg = new QLabel(this);

                bg->move(0, 0);
                bg->setPixmap(*background);
                bg->lower();
                m_qWidgetList.append(bg);
                this->setFixedSize(background->width(),background->height());
		parent->setMinimumSize(background->width(), background->height());
                this->move(0,0);
                if (!WWidget::selectNode(node, "BgColor").isNull()) {
                    c.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
                }

                QPalette palette;
                palette.setBrush(QPalette::Window, WSkinColor::getCorrectColor(c));
                parent->setBackgroundRole(QPalette::Window);
                parent->setPalette(palette);
                //parent->setEraseColor(WSkinColor::getCorrectColor(c));
                parent->setAutoFillBackground(true);

    //Next, let's set up the colour palette for Mixxx's non-skinned elements
    //(eg. search box, combobox, toolbar)
    //For what color controls what, see this reference:
    //http://doc.trolltech.com/4.3/qpalette.html

    //palette.setColor(QPalette::Text, QColor("white")); //combobox and search box text colour
    //palette.setColor(QPalette::WindowText, QColor("white"));
    //palette.setColor(QPalette::Base, WSkinColor::getCorrectColor(c)); //search box background colour
    //palette.setColor(QPalette::Button, WSkinColor::getCorrectColor(c));
    //parent->setPalette(palette);

            }
            else if (node.nodeName()=="VuMeter")
            {
                WVuMeter * p = new WVuMeter(this);
                m_qWidgetList.append(p);
                p->setup(node);
                p->installEventFilter(m_pKeyboard);
            }
            else if (node.nodeName()=="StatusLight")
            {
                WStatusLight * p = new WStatusLight(this);
                m_qWidgetList.append(p);
                p->setup(node);
                p->installEventFilter(m_pKeyboard);
            }
            else if (node.nodeName()=="Visual")
            {
                WaveformViewerType type;

                if (WWidget::selectNodeInt(node, "Channel")==1)
                {
                    type = WaveformViewerFactory::createWaveformViewer("[Channel1]", this, pConfig, &m_pVisualCh1, m_pWaveformRendererCh1);
                    connect(&m_guiTimer, SIGNAL(timeout()), m_pVisualCh1, SLOT(refresh()));
                    m_qWidgetList.append(m_pVisualCh1);

                    m_pVisualCh1->installEventFilter(m_pKeyboard);

                    // Hook up [Channel1],wheel Control Object to the Visual Controller
                    ControlObjectThreadWidget * p = new ControlObjectThreadWidget(ControlObject::getControl(ConfigKey("[Channel1]", "wheel")));
                    p->setWidget((QWidget *)m_pVisualCh1, true, true, true, Qt::LeftButton);

                    //ControlObject::setWidget((QWidget *)m_pVisualCh1, ConfigKey("[Channel1]", "wheel"), true, Qt::LeftButton);

                    // Things to do whether the waveform was previously created or not
                    if(type == WAVEFORM_GL) {
                        m_bVisualWaveform = true; // TODO : remove this crust
                        ((WGLWaveformViewer*)m_pVisualCh1)->setup(node);
                        // TODO rryan re-enable this later
                        /*
                          ((WVisualWaveform*)m_pVisualCh1)->resetColors();
                        */
                    } else if (type == WAVEFORM_WIDGET) {
                        m_bVisualWaveform = true;
                        ((WWaveformViewer *)m_pVisualCh1)->setup(node);
                    } else if (type == WAVEFORM_SIMPLE) {
                        ((WVisualSimple*)m_pVisualCh1)->setup(node);
                    }
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                {
                    type = WaveformViewerFactory::createWaveformViewer("[Channel2]", this, pConfig, &m_pVisualCh2, m_pWaveformRendererCh2);
                    connect(&m_guiTimer, SIGNAL(timeout()), m_pVisualCh2, SLOT(refresh()));
                    m_qWidgetList.append(m_pVisualCh2);

                    m_pVisualCh2->installEventFilter(m_pKeyboard);

                    // Hook up [Channel1],wheel Control Object to the Visual Controller
                    ControlObjectThreadWidget * p = new ControlObjectThreadWidget(ControlObject::getControl(ConfigKey("[Channel2]", "wheel")));
                    p->setWidget((QWidget *)m_pVisualCh2, true, true, true, Qt::LeftButton);

                    //ControlObject::setWidget((QWidget *)m_pVisualCh2, ConfigKey("[Channel2]", "wheel"), true, Qt::LeftButton);

                    // Things to do whether the waveform was previously created or not
                    if(type == WAVEFORM_GL) {
                        m_bVisualWaveform = true; // TODO : remove this crust

                        ((WGLWaveformViewer*)m_pVisualCh2)->setup(node);
                        // TODO rryan re-enable this later
                        /*
                          ((WVisualWaveform*)m_pVisualCh2)->resetColors();
                        */
                    } else if (type == WAVEFORM_WIDGET) {
                        m_bVisualWaveform = true;
                        ((WWaveformViewer *)m_pVisualCh2)->setup(node);
                    } else if (type == WAVEFORM_SIMPLE) {
                        ((WVisualSimple*)m_pVisualCh2)->setup(node);
                    }
                }
            }

            /*############## PERSISTENT OBJECT ##############*/


            // persistent: m_pTextCh1, m_pTextCh2

            // NOTE: The m_pTextCh1/2 widgets are no longer persistent, but they
            // are stored in those member fields. -- rryan 8/2010
            else if (node.nodeName()=="Text")
            {
                QLabel * p = 0;

                if (m_pTextCh1 != NULL || m_pTextCh2 != NULL) {
                    qDebug() << "WARNING: Text widgets not destroyed before rebooting GUI.";
                }

                p = new QLabel(this);
                p->installEventFilter(m_pKeyboard);
                m_qWidgetList.append(p);

                // Associate pointers
                if (WWidget::selectNodeInt(node, "Channel")==1) {
                    m_pTextCh1 = p;
                    m_pTextCh1->setText(m_textCh1);
                } else if (WWidget::selectNodeInt(node, "Channel")==2) {
                    m_pTextCh2 = p;
                    m_pTextCh2->setText(m_textCh2);
                }

                // Set position
                QString pos = WWidget::selectNodeQString(node, "Pos");
                int x = pos.left(pos.indexOf(",")).toInt();
                int y = pos.mid(pos.indexOf(",")+1).toInt();
                p->move(x,y);

                // Get tooltip
                QString tooltip = WWidget::selectNodeQString(node, "Tooltip");
                p->setToolTip(tooltip);

                // Size
                QString size = WWidget::selectNodeQString(node, "Size");
                x = size.left(size.indexOf(",")).toInt();
                y = size.mid(size.indexOf(",")+1).toInt();
                p->setFixedSize(x,y);

                // Background color
                QColor bgc(255,255,255);
                if (!WWidget::selectNode(node, "BgColor").isNull()) {
                    bgc.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
                    p->setAutoFillBackground(true);
                }
                //p->setPaletteBackgroundColor(WSkinColor::getCorrectColor(bgc));
                QPalette palette;
                palette.setBrush(p->backgroundRole(), WSkinColor::getCorrectColor(bgc));
                p->setPalette(palette);

                // Foreground color
                QColor fgc(0,0,0);
                if (!WWidget::selectNode(node, "FgColor").isNull()) {
                    fgc.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
                }
		//                p->setPaletteForegroundColor(WSkinColor::getCorrectColor(fgc));
                //QPalette palette;
                palette.setBrush(p->foregroundRole(), WSkinColor::getCorrectColor(fgc));
                p->setPalette(palette);

                QString style = WWidget::selectNodeQString(node, "Style");
                if (style != "") {
                    p->setStyleSheet(style);
                }

                // Alignment
                if (!WWidget::selectNode(node, "Align").isNull() && WWidget::selectNodeQString(node, "Align")=="right")
                    p->setAlignment(Qt::AlignRight);

                p->show();
            }

            // persistent: m_pNumberPosCh1, m_pNumberPosCh2

            // NOTE: The m_pNumberPosCh1/2 widgets are no longer persistent, but they
            // are stored in those member fields. -- rryan 8/2010
            else if (node.nodeName()=="NumberPos")
            {
                if (m_pNumberPosCh1 != NULL || m_pNumberPosCh2 != NULL) {
                    qDebug() << "WARNING: Number widgets were not reset before rebooting GUI.";
                }

                if (WWidget::selectNodeInt(node, "Channel")==1) {
                    m_pNumberPosCh1 = new WNumberPos("[Channel1]", this);
                    m_pNumberPosCh1->installEventFilter(m_pKeyboard);
                    m_pNumberPosCh1->setup(node);
                    m_pNumberPosCh1->setRemain(m_bDurationRemain);
                    m_pNumberPosCh1->show();
                    m_qWidgetList.append(m_pNumberPosCh1);
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2) {
                    m_pNumberPosCh2 = new WNumberPos("[Channel2]", this);
                    m_pNumberPosCh2->installEventFilter(m_pKeyboard);
                    m_pNumberPosCh2->setup(node);
                    m_pNumberPosCh2->setRemain(m_bDurationRemain);
                    m_pNumberPosCh2->show();
                    m_qWidgetList.append(m_pNumberPosCh2);
                }
            }

            // persistent: m_pSliderRateCh1, m_pSliderRateCh2
            else if (node.nodeName()=="SliderComposed")
            {
                // If rate slider...
                if (compareConfigKeys(node, "[Channel1],rate")) {
                    if (m_pSliderRateCh1 == 0) {
                        m_pSliderRateCh1 = new WSliderComposed(this);
			m_pSliderRateCh1->installEventFilter(m_pKeyboard);
                    }
                    m_pSliderRateCh1->setup(node);
		    m_pSliderRateCh1->show();
                }
                else if (compareConfigKeys(node, "[Channel2],rate")) {
                    if (m_pSliderRateCh2 == 0) {
                        m_pSliderRateCh2 = new WSliderComposed(this);
                        m_pSliderRateCh2->installEventFilter(m_pKeyboard);
                    }
		    m_pSliderRateCh2->setup(node);
		    m_pSliderRateCh2->show();
                } else {
		    WSliderComposed * p = new WSliderComposed(this);
		    p->setup(node);
		    p->installEventFilter(m_pKeyboard);
		    m_qWidgetList.append(p);
		    currentControl = qobject_cast<WAbstractControl*>(p);
		}
            }

            // persistent: m_pOverviewCh1, m_pOverviewCh2
            else if (node.nodeName()=="Overview")
            {
                if (WWidget::selectNodeInt(node, "Channel")==1)
                {
                    if (m_pOverviewCh1 == 0) {
                        m_pOverviewCh1 = new WOverview("[Channel1]", this);
                        //m_qWidgetList.append(m_pOverviewCh1);

                    }
                    m_pOverviewCh1->setup(node);
		    m_pOverviewCh1->show();
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                {
                    if (m_pOverviewCh2 == 0) {
                        m_pOverviewCh2 = new WOverview("[Channel2]", this);
                        //m_qWidgetList.append(m_pOverviewCh2);
                    }
                    m_pOverviewCh2->setup(node);
		    m_pOverviewCh2->show();
                }
            }

            /*
            // persistent: m_pLineEditSearch
            else if (node.nodeName()=="Search")
            {
                if (m_pLineEditSearch == 0) {
		    MixxxView* parent = this;
                    QString path = pConfig->getConfigPath();
                    m_pLineEditSearch = new WSearchLineEdit(path, this);
                    m_pLibraryPageLayout->addWidget(m_pLineEditSearch, 0, 2, Qt::AlignRight); //Row 0, col 2
                }

                // Size
                QString size = WWidget::selectNodeQString(node, "Size");
                int x = size.left(size.indexOf(",")).toInt();
                int y = size.mid(size.indexOf(",")+1).toInt();
                m_pLineEditSearch->setFixedSize(x,y);
                m_pLineEditSearch->show();
            }
            */

	    // persistent: m_pTabWidget
            else if (node.nodeName()=="TableView")
            {
                if (m_pTabWidget == 0) {
                    // If the tab widget is NULL than we cannot have possibly
                    // set anything else up, so instead of having awkward
                    // separation of creation of each thing, lets just merge
                    // them together here, assuming nothing has been created
                    // yet.

                    //Create the tab widget to store the various panes in
                    //(library, effects, etc.)
                    m_pTabWidget = new QStackedWidget(this);

                    // Create the pages that go in the tab widget
                    m_pTabWidgetLibraryPage = new QWidget(this);
#ifdef __LADSPA__
                    m_pLADSPAView = new LADSPAView(this);
                    m_pTabWidgetEffectsPage = m_pLADSPAView;
#else
                    m_pTabWidgetEffectsPage = new QWidget();
#endif

                    //Set the margins to be 0 for all the layouts.
                    m_pLibraryPageLayout->setContentsMargins(0, 0, 0, 0);
#ifdef __LADSPA__
//                     m_pEffectsPageLayout->setContentsMargins(0, 0, 0, 0);
#endif

                    m_pTabWidgetLibraryPage->setLayout(m_pLibraryPageLayout);
                    //m_pTabWidgetEffectsPage->setLayout(m_pEffectsPageLayout);

                    //Set up the search box widget
                    if (m_pLineEditSearch == 0) {
                        QString path = pConfig->getConfigPath();
                        m_pLineEditSearch = new WSearchLineEdit(path, node, this);
                        //m_pLibraryPageLayout->addWidget(m_pLineEditSearch, 0, 2, Qt::AlignRight); //Row 0, col 2
                        //m_pLineEditSearch->show();

                        // Size
                        /*
                        QString size = WWidget::selectNodeQString(node, "Size");
                        int x = size.left(size.indexOf(",")).toInt();
                        int y = size.mid(size.indexOf(",")+1).toInt();
                        m_pLineEditSearch->setFixedSize(x,y);
                        */
                    }



                    // Build the Library widgets
                    m_pSplitter = new QSplitter(m_pTabWidgetLibraryPage);

                    m_pLibraryWidget = new WLibrary(m_pSplitter);
                    m_pLibraryWidget->installEventFilter(m_pKeyboard);


                    m_pLibrarySidebar = new WLibrarySidebar(m_pSplitter);
                    m_pLibrarySidebar->installEventFilter(m_pKeyboard);

                    m_pLibrarySidebarPage = new QWidget(m_pSplitter);
                    QVBoxLayout* vl = new QVBoxLayout();
                    vl->setContentsMargins(0,0,0,0); //Fill entire space
                    m_pLibrarySidebarPage->setLayout(vl);
                    vl->addWidget(m_pLineEditSearch);
                    vl->addWidget(m_pLibrarySidebar);

                    setupTrackSourceViewWidget(node);

                    m_pLibrary->bindWidget(m_pLibrarySidebar,
                                           m_pLibraryWidget,
                                           m_pKeyboard);

                    //Add the library sidebar to the splitter.
                    m_pSplitter->addWidget(m_pLibrarySidebarPage);
                    //Add the library widget to the splitter.
                    m_pSplitter->addWidget(m_pLibraryWidget);

                    QString style = WWidget::selectNodeQString(node, "Style");
                    if (style != "") {
                        m_pTabWidget->setStyleSheet(style);
                    }

                    // TODO(rryan) can we make this more elegant?
                    QList<int> splitterSizes;
                    splitterSizes.push_back(50);
                    splitterSizes.push_back(500);
                    m_pSplitter->setSizes(splitterSizes);

                    // Add the splitter to the library page's layout, so it's
                    // positioned/sized automatically
                    m_pLibraryPageLayout->addWidget(m_pSplitter,
                                                            1, 0, //From row 1, col 0,
                                                            1,    //Span 1 row
                                                            3,    //Span 3 cols
                                                            0);   //Default alignment

                    // TODO(XXX) Re-enable this to get the tab widget back, post
                    // 1.7.0 release.

                    // Add the library page to the tab widget.
                    //m_pTabWidget->addTab(m_pTabWidgetLibraryPage, tr("Library"));
                    m_pTabWidget->addWidget(m_pTabWidgetLibraryPage);

                    // Add the effects page to the tab widget.
                    //m_pTabWidget->addTab(m_pTabWidgetEffectsPage, tr("Effects"));
                    m_pTabWidget->addWidget(m_pTabWidgetEffectsPage);
                }

                //Move the tab widget into position and size it properly.
                setupTabWidget(node);

                setupTrackSourceViewWidget(node);

                // Applies the node settings to every view registered in the
                // Library widget.
                m_pLibraryWidget->setup(node);

                m_pLineEditSearch->setup(node);

                m_pLineEditSearch->show();
                m_pTabWidget->show();
            }
            // set default value (only if it changes from the standard value)
            if (currentControl) {
                if (compareConfigKeys(node, "[Master],headMix"))
                {
                    currentControl->setDefaultValue(0.);
                }
                else if (compareConfigKeys(node, "[Channel1],volume")||
                         compareConfigKeys(node, "[Channel2],volume"))
                {
                    currentControl->setDefaultValue(127.);
                }
            }
        }
        node = node.nextSibling();
    }
}


void MixxxView::rebootGUI(QWidget * parent, ConfigObject<ConfigValue> * pConfig, QString qSkinPath) {
    QObject *obj;
    int i;

    // This isn't thread safe, so this must only be called from Qt main thread.


    // Clear the pointers to widgets that are going to be deleted (e.g. on the
    // widget list) but we retain pointers to for contacting from other parts of
    // Mixxx.
    m_pVisualCh1 = NULL;
    m_pVisualCh2 = NULL;
    m_pTextCh1 = NULL;
    m_pTextCh2 = NULL;
    m_pNumberPosCh1 = NULL;
    m_pNumberPosCh2 = NULL;
    m_pNumberBpmCh1 = NULL;
    m_pNumberBpmCh2 = NULL;

    // Delete all the widgets on the 'widget list'. This is a list of active
    // screen widgets that should be automatically deleted on a skin reboot or
    // shutdown.
    while (!m_qWidgetList.isEmpty()) {
        delete m_qWidgetList.takeFirst();
    }

    // Hide all widgets that are persistent across GUI reboots.
    if (m_pSliderRateCh1) m_pSliderRateCh1->hide();
    if (m_pSliderRateCh2) m_pSliderRateCh2->hide();
    if (m_pOverviewCh1) m_pOverviewCh1->hide();
    if (m_pOverviewCh2) m_pOverviewCh2->hide();
    if (m_pLineEditSearch) m_pLineEditSearch->hide();
    if (m_pTabWidget) m_pTabWidget->hide();

    //load the skin
    QDomElement docElem = openSkin(qSkinPath);
    setupColorScheme(docElem, pConfig);
    createAllWidgets(docElem, parent, pConfig);
    show();

    for (i = 0; i < m_qWidgetList.size(); ++i) {
        obj = m_qWidgetList[i];
        ((QWidget *)obj)->show();
    }
}


QList<QString> MixxxView::getSchemeList(QString qSkinPath) {

    QDomElement docElem = openSkin(qSkinPath);
    QList<QString> schlist;

    QDomNode colsch = docElem.namedItem("Schemes");
    if (!colsch.isNull() && colsch.isElement()) {
        QDomNode sch = colsch.firstChild();

        while (!sch.isNull()) {
            QString thisname = WWidget::selectNodeQString(sch, "Name");
            schlist.append(thisname);
            sch = sch.nextSibling();
        }
    }

    return schlist;
}

void MixxxView::setupTabWidget(QDomNode node)
{
    // Position
    if (!WWidget::selectNode(node, "Pos").isNull())
    {
        QString pos = WWidget::selectNodeQString(node, "Pos");
        int x = pos.left(pos.indexOf(",")).toInt();
        int y = pos.mid(pos.indexOf(",")+1).toInt();
        m_pTabWidget->move(x,y);
    }

    // Size
    if (!WWidget::selectNode(node, "Size").isNull())
    {
        QString size = WWidget::selectNodeQString(node, "Size");
        int x = size.left(size.indexOf(",")).toInt();
        int y = size.mid(size.indexOf(",")+1).toInt();
        m_pTabWidget->setFixedSize(x,y);
    }
}


void MixxxView::setupTrackSourceViewWidget(QDomNode node)
{

    //Setup colors:
    //Foreground color
    QColor fgc(0,255,0);
    if (!WWidget::selectNode(node, "FgColor").isNull()) {

	fgc.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));

	//m_pLibrarySidebar->setForegroundColor(WSkinColor::getCorrectColor(fgc));

	// Row colors
	if (!WWidget::selectNode(node, "BgColorRowEven").isNull())
	    {
	        QColor r1;
	        r1.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowEven"));
		r1 = WSkinColor::getCorrectColor(r1);
		QColor r2;
		r2.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowUneven"));
		r2 = WSkinColor::getCorrectColor(r2);

		// For now make text the inverse of the background so it's readable
		// In the future this should be configurable from the skin with this
		// as the fallback option
		QColor text(255 - r1.red(), 255 - r1.green(), 255 - r1.blue());

	        QPalette Rowpalette = palette();
	        Rowpalette.setColor(QPalette::Base, r1);
	        Rowpalette.setColor(QPalette::AlternateBase, r2);
		Rowpalette.setColor(QPalette::Text, text);

	        m_pLibrarySidebar->setPalette(Rowpalette);
	    }
    }

}

void MixxxView::slotSetupTrackConnectionsCh1(TrackPointer pTrack)
{
    //Note: This slot gets called when Player emits a newTrackLoaded() signal.
    //Connect the track to the waveform overview widget, so it updates when the
    //wavesummary is finished generating.
    connect(pTrack.data(), SIGNAL(wavesummaryUpdated(TrackInfoObject*)),
            m_pOverviewCh1, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
    //Connect the track to the BPM readout in the GUI, so it updates when the
    //BPM is finished being calculated.
    connect(pTrack.data(), SIGNAL(bpmUpdated(double)),
            m_pNumberBpmCh1, SLOT(setValue(double)));
}

void MixxxView::slotSetupTrackConnectionsCh2(TrackPointer pTrack)
{
    //Note: This slot gets called when Player emits a newTrackLoaded() signal.
    //Connect the track to the waveform overview widget, so it updates when the
    //wavesummary is finished generating.
    connect(pTrack.data(), SIGNAL(wavesummaryUpdated(TrackInfoObject*)),
            m_pOverviewCh2, SLOT(slotLoadNewWaveform(TrackInfoObject*)));
    //Connect the track to the BPM readout in the GUI, so it updates when the
    //BPM is finished being calculated.
    connect(pTrack.data(), SIGNAL(bpmUpdated(double)),
            m_pNumberBpmCh2, SLOT(setValue(double)));

}

void MixxxView::slotUpdateTrackTextCh1(TrackPointer pTrack)
{
    m_textCh1 = pTrack->getInfo();
    if (m_pTextCh1)
        m_pTextCh1->setText(m_textCh1);
}

void MixxxView::slotUpdateTrackTextCh2(TrackPointer pTrack)
{
    m_textCh2 = pTrack->getInfo();
    if (m_pTextCh2)
        m_pTextCh2->setText(m_textCh2);
}

void MixxxView::slotClearTrackTextCh1(TrackPointer pTrack)
{
    m_textCh1 = "";
    if (m_pTextCh1)
        m_pTextCh1->setText("");
}

void MixxxView::slotClearTrackTextCh2(TrackPointer pTrack)
{
    m_textCh2 = "";
    if (m_pTextCh2)
        m_pTextCh2->setText("");
}

void MixxxView::slotSetDurationRemaining(bool bDurationRemaining) {
    m_bDurationRemain = bDurationRemaining;
    if (m_pNumberPosCh1)
        m_pNumberPosCh1->setRemain(m_bDurationRemain);
    if (m_pNumberPosCh2)
        m_pNumberPosCh2->setRemain(m_bDurationRemain);
}
