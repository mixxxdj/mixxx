/* -*- mode:C++; indent-tabs-mode:t; tab-width:8; c-basic-offset:4; -*- */
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

#include "mixxxview.h"

#include <QDir>
#include <QPixmap>
#include <QToolTip>
#include <QEvent>
#include <QMenuBar>
#include <QList>
#include <QTableView>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QTableView>
#include <QLineEdit>
#include <QtDebug>
#include <QMessageBox>

#include "wtracktablemodel.h"
#include "wtracktableview.h"
#include "wwidget.h"
#include "wknob.h"
#include "wpushbutton.h"
#include "wslider.h"
#include "wslidercomposed.h"
#include "wdisplay.h"
#include "wvumeter.h"
#include "wstatuslight.h"
#include "wlabel.h"
#include "wnumber.h"
#include "wnumberpos.h"
#include "wnumberbpm.h"
#include "wnumberrate.h"
#include "wpixmapstore.h"
#include "wvisualwaveform.h"
#include "wvisualsimple.h"
#include "woverview.h"
#include "mixxxkeyboard.h"
#include "controlobject.h"
#include "controlobjectthreadwidget.h"

#include "imgloader.h"
#include "imginvert.h"
#include "imgcolor.h"
#include "wskincolor.h"
#include "mixxx.h"

MixxxView::MixxxView(QWidget * parent, ConfigObject<ConfigValueKbd> * kbdconfig, bool bVisualsWaveform, QString qSkinPath, ConfigObject<ConfigValue> * pConfig) : QWidget(parent)
{
    view = 0;
    //    m_qWidgetList.setAutoDelete(true);
    m_pconfig = pConfig;

    m_pKeyboard = new MixxxKeyboard(kbdconfig);
    installEventFilter(m_pKeyboard);

    // Try to open the file pointed to by qSkinPath
    QDomElement docElem = openSkin(qSkinPath);

#ifdef __WIN__
#ifndef QT3_SUPPORT
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::MemoryOptim);
#endif
#endif

    // Default values for visuals
    //m_pTrackTable = 0;
    m_pTextCh1 = 0;
    m_pTextCh2 = 0;
    m_pVisualCh1 = 0;
    m_pVisualCh2 = 0;
    m_pNumberPosCh1 = 0;
    m_pNumberPosCh2 = 0;
    m_pSliderRateCh1 = 0;
    m_pSliderRateCh2 = 0;
    m_bZoom = false;
    m_bVisualWaveform = false;
    m_pOverviewCh1 = 0;
    m_pOverviewCh2 = 0;
    m_pComboBox = 0;
    m_pTrackTableView = 0;
    m_pLineEditSearch = 0;

    setupColorScheme(docElem, pConfig);

    // Load all widgets defined in the XML file
    createAllWidgets(docElem, parent, bVisualsWaveform, pConfig);

#ifdef __WIN__
#ifndef QT3_SUPPORT
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::NormalOptim);
#endif
#endif

}

MixxxView::~MixxxView()
{
    //m_qWidgetList.clear();
}

void MixxxView::checkDirectRendering()
{
    // Check if DirectRendering is enabled and display warning
    if ((m_pVisualCh1 && !((WVisualWaveform *)m_pVisualCh1)->directRendering()) ||
        (m_pVisualCh2 && !((WVisualWaveform *)m_pVisualCh2)->directRendering()))
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

    if (!file.open(QIODevice::ReadOnly))
    {
	qFatal("Could not open skin definition file: %s", (char *)fi.fileName().constData());
    }
    if (!skin.setContent(&file))
    {
	qFatal("Error parsing skin definition file: %s", (char *)fi.fileName().constData());
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
                                 bool bVisualsWaveform,
                                 ConfigObject<ConfigValue> * pConfig) {

    QDomNode node = docElem.firstChild();
    while (!node.isNull())
    {
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
                if (WWidget::selectNodeInt(node, "Channel")==1)
                {
                    WNumberBpm * p = new WNumberBpm("[Channel1]", this);
                    p->setup(node);
                    p->installEventFilter(m_pKeyboard);
                    m_qWidgetList.append(p);
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                {
                    WNumberBpm * p = new WNumberBpm("[Channel2]", this);
                    p->setup(node);
                    p->installEventFilter(m_pKeyboard);
                    m_qWidgetList.append(p);
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
                    //p->setBackgroundRole(QPalette::Window);
                    p->setPalette(palette);
                    p->setAutoFillBackground(true);


                }
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                {
                    WNumberRate * p = new WNumberRate("[Channel2]", this);
                    p->setup(node);
                    p->installEventFilter(m_pKeyboard);
                    m_qWidgetList.append(p);
                    //p->setBackgroundRole(QPalette::Window);
                    p->setPalette(palette);
                    p->setAutoFillBackground(true);
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
                QColor c(255,255,255);
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







            /*############## PERSISTENT OBJECT ##############*/
            // persistent: m_pTextCh1, m_pTextCh2
            else if (node.nodeName()=="Text")
            {
                QLabel * p = 0;
                // Associate pointers
                if (WWidget::selectNodeInt(node, "Channel")==1 && m_pTextCh1 != 0)
                    p = m_pTextCh1;
                else if (WWidget::selectNodeInt(node, "Channel")==2  && m_pTextCh2 != 0)
                    p = m_pTextCh2;
                else {
                    p = new QLabel(this);
                    p->installEventFilter(m_pKeyboard);
                }

                // Associate pointers
                if (WWidget::selectNodeInt(node, "Channel")==1)
                    m_pTextCh1 = p;
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                    m_pTextCh2 = p;
		else
                    m_qWidgetList.append(p);

                // Set position
                QString pos = WWidget::selectNodeQString(node, "Pos");
                int x = pos.left(pos.indexOf(",")).toInt();
                int y = pos.mid(pos.indexOf(",")+1).toInt();
                p->move(x,y);

                // Size
                QString size = WWidget::selectNodeQString(node, "Size");
                x = size.left(size.indexOf(",")).toInt();
                y = size.mid(size.indexOf(",")+1).toInt();
                p->setFixedSize(x,y);

                // Background color
                QColor bgc(255,255,255);
                if (!WWidget::selectNode(node, "BgColor").isNull()) {
                    bgc.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
                }
                //p->setPaletteBackgroundColor(WSkinColor::getCorrectColor(bgc));
                QPalette palette;
                palette.setBrush(p->backgroundRole(), WSkinColor::getCorrectColor(bgc));
                p->setPalette(palette);
                p->setAutoFillBackground(true);

                // Foreground color
                QColor fgc(0,0,0);
                if (!WWidget::selectNode(node, "FgColor").isNull()) {
                    fgc.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
                }
		//                p->setPaletteForegroundColor(WSkinColor::getCorrectColor(fgc));
                //QPalette palette;
                palette.setBrush(p->foregroundRole(), WSkinColor::getCorrectColor(fgc));
                p->setPalette(palette);

                // Alignment
                if (!WWidget::selectNode(node, "Align").isNull() && WWidget::selectNodeQString(node, "Align")=="right")
                    p->setAlignment(Qt::AlignRight);

		p->show();
            }



            // persistent: m_pVisualCh1, m_pVisualCh2
            else if (node.nodeName()=="Visual")
            {
                if (WWidget::selectNodeInt(node, "Channel")==1 && m_pVisualCh1!=0) {
                    ((WVisualWaveform *)m_pVisualCh1)->setup(node);
                    ((WVisualWaveform *)m_pVisualCh1)->resetColors();
                    ((WVisualWaveform *)m_pVisualCh1)->show();
                    ((WVisualWaveform *)m_pVisualCh1)->repaint();
                }
                else if (WWidget::selectNodeInt(node, "Channel")==1 && m_pVisualCh1==0)
                {
                    if (bVisualsWaveform)
                    {
                        m_pVisualCh1 = new WVisualWaveform(this, (QGLWidget *)m_pVisualCh2);
                        if (((WVisualWaveform *)m_pVisualCh1)->isValid())
                        {
                            ((WVisualWaveform *)m_pVisualCh1)->setup(node);
                            m_pVisualCh1->installEventFilter(m_pKeyboard);
                            m_bVisualWaveform = true;
                        }
                        else
                        {
                            m_bVisualWaveform = false;
                            delete m_pVisualCh1;
			    m_pVisualCh1 = 0;
                        }
                    }
                    if (!m_bVisualWaveform)
                    {
                        m_pVisualCh1 = new WVisualSimple(this, 0);
                        ((WVisualSimple *)m_pVisualCh1)->setup(node);
                        m_pVisualCh1->installEventFilter(m_pKeyboard);
                    }
                    ControlObjectThreadWidget * p = new ControlObjectThreadWidget(ControlObject::getControl(ConfigKey("[Channel1]", "wheel")));
                    p->setWidget((QWidget *)m_pVisualCh1, true, Qt::LeftButton);
                    //ControlObject::setWidget((QWidget *)m_pVisualCh1, ConfigKey("[Channel1]", "wheel"), true, Qt::LeftButton);
                }

                if (WWidget::selectNodeInt(node, "Channel")==2 && m_pVisualCh2!=0) {
                    ((WVisualWaveform *)m_pVisualCh2)->setup(node);
                    ((WVisualWaveform *)m_pVisualCh2)->resetColors();
                    ((WVisualWaveform *)m_pVisualCh2)->show();
                    ((WVisualWaveform *)m_pVisualCh2)->repaint();
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2 && m_pVisualCh2==0)
                {
                    if (bVisualsWaveform)
                    {
                        m_pVisualCh2 = new WVisualWaveform(this, (QGLWidget *)m_pVisualCh1);
                        if (((WVisualWaveform *)m_pVisualCh2)->isValid())
                        {
                            ((WVisualWaveform *)m_pVisualCh2)->setup(node);
                            m_pVisualCh2->installEventFilter(m_pKeyboard);
                            m_bVisualWaveform = true;
                        }
                        else
                        {
                            m_bVisualWaveform = false;
                            delete m_pVisualCh2;
			    m_pVisualCh2 = 0;
                        }
                    }
                    if (!m_bVisualWaveform)
                    {
                        m_pVisualCh2 = new WVisualSimple(this, 0);
                        ((WVisualSimple *)m_pVisualCh2)->setup(node);
                        m_pVisualCh2->installEventFilter(m_pKeyboard);
                    }
                    ControlObjectThreadWidget * p = new ControlObjectThreadWidget(ControlObject::getControl(ConfigKey("[Channel2]", "wheel")));
                    p->setWidget((QWidget *)m_pVisualCh2, true, Qt::LeftButton);
                    //ControlObject::setWidget((QWidget *)m_pVisualCh2, ConfigKey("[Channel2]", "wheel"), true, Qt::LeftButton);
                }

                if (!WWidget::selectNode(node, "Zoom").isNull() && WWidget::selectNodeQString(node, "Zoom")=="true")
                    m_bZoom = true;
            }




            // persistent: m_pNumberPosCh1, m_pNumberPosCh2
            else if (node.nodeName()=="NumberPos")
            {
                if (WWidget::selectNodeInt(node, "Channel")==1)
                {
                    if (m_pNumberPosCh1 == 0) {
                        m_pNumberPosCh1 = new WNumberPos("[Channel1]", this);
                        m_pNumberPosCh1->installEventFilter(m_pKeyboard);
                    }
		    m_pNumberPosCh1->setup(node);
		    m_pNumberPosCh1->show();

                }
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                {
                    if (m_pNumberPosCh2 == 0) {
                        m_pNumberPosCh2 = new WNumberPos("[Channel2]", this);
                        m_pNumberPosCh2->installEventFilter(m_pKeyboard);
		    }
		    m_pNumberPosCh2->setup(node);
		    m_pNumberPosCh2->show();
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
		}

            }



            // persistent: m_pOverviewCh1, m_pOverviewCh2
            else if (node.nodeName()=="Overview")
            {
                if (WWidget::selectNodeInt(node, "Channel")==1)
                {
                    if (m_pOverviewCh1 == 0) {
                        m_pOverviewCh1 = new WOverview(this);
                        //m_qWidgetList.append(m_pOverviewCh1);
                    }
                    m_pOverviewCh1->setup(node);
		    m_pOverviewCh1->show();
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                {
                    if (m_pOverviewCh2 == 0) {
                        m_pOverviewCh2 = new WOverview(this);
                        //m_qWidgetList.append(m_pOverviewCh2);
                    }
                    m_pOverviewCh2->setup(node);
		    m_pOverviewCh2->show();
                }
            }

	    // persistent: m_pComboBox
            else if (node.nodeName()=="ComboBox")
            {
                if (m_pComboBox == 0) {
		    m_pComboBox = new QComboBox(this);
		    m_pComboBox->addItem( "Library" );
		    m_pComboBox->addItem( "Play Queue" );
		    m_pComboBox->addItem( "Browse" );
		    m_pComboBox->addItem( "Playlists" );
		}
                // Set position
                QString pos = WWidget::selectNodeQString(node, "Pos");
                int x = pos.left(pos.indexOf(",")).toInt();
                int y = pos.mid(pos.indexOf(",")+1).toInt();
                m_pComboBox->move(x,y);

                // Size
                QString size = WWidget::selectNodeQString(node, "Size");
                x = size.left(size.indexOf(",")).toInt();
                y = size.mid(size.indexOf(",")+1).toInt();
                m_pComboBox->setFixedSize(x,y);
                m_pComboBox->show();
            }

	    // persistent: m_pLineEditSearch
            else if (node.nodeName()=="Search")
            {
		if (m_pLineEditSearch == 0) {
		    m_pLineEditSearch = new QLineEdit(this);
		}

                // Set position
                QString pos = WWidget::selectNodeQString(node, "Pos");
                int x = pos.left(pos.indexOf(",")).toInt();
                int y = pos.mid(pos.indexOf(",")+1).toInt();
                m_pLineEditSearch->move(x+35,y);

                // Size
                QString size = WWidget::selectNodeQString(node, "Size");
                x = size.left(size.indexOf(",")).toInt();
                y = size.mid(size.indexOf(",")+1).toInt();
                m_pLineEditSearch->setFixedSize(x,y);
		m_pLineEditSearch->show();
            }

	    // persistent: m_pTrackTableView
            else if (node.nodeName()=="TableView")
            {
		if (m_pTrackTableView == 0) {
		    m_pTrackTableView = new WTrackTableView(this, pConfig);
		}
                m_pTrackTableView->setup(node);
		m_pTrackTableView->show();
            }
        }
        node = node.nextSibling();

    }
}


void MixxxView::rebootGUI(QWidget * parent, bool bVisualsWaveform, ConfigObject<ConfigValue> * pConfig, QString qSkinPath) {
    QObject *obj;
    int i;

    // This isn't thread safe, does anything else hack on this object?

    //remove all widget from the list (except permanent one)
    while (!m_qWidgetList.isEmpty()) {
	delete m_qWidgetList.takeFirst();
    }

    //hide permanent widget
    if (m_pTextCh1) m_pTextCh1->hide();
    if (m_pTextCh2) m_pTextCh2->hide();
    if (m_pVisualCh1) ((QWidget *)m_pVisualCh1)->hide();
    if (m_pVisualCh2) ((QWidget *)m_pVisualCh2)->hide();
    if (m_pNumberPosCh1) m_pNumberPosCh1->hide();
    if (m_pNumberPosCh2) m_pNumberPosCh2->hide();
    if (m_pSliderRateCh1) m_pSliderRateCh1->hide();
    if (m_pSliderRateCh2) m_pSliderRateCh2->hide();
    if (m_pOverviewCh1) m_pOverviewCh1->hide();
    if (m_pOverviewCh2) m_pOverviewCh2->hide();
    if (m_pComboBox) m_pComboBox->hide();
    if (m_pLineEditSearch) m_pLineEditSearch->hide();
    if (m_pTrackTableView) m_pTrackTableView->hide();

    //load the skin
    QDomElement docElem = openSkin(qSkinPath);
    setupColorScheme(docElem, pConfig);
    createAllWidgets(docElem, parent, bVisualsWaveform, pConfig);
    show();

    for (i = 0; i < m_qWidgetList.size(); ++i) {
	obj = m_qWidgetList[i];
	((QWidget *)obj)->show();
    }
    if (m_pVisualCh1) ((QWidget *)m_pVisualCh1)->repaint();
    if (m_pVisualCh2) ((QWidget *)m_pVisualCh2)->repaint();
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
