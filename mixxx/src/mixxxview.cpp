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

#include <qtable.h>
#include <qdir.h>
#include <qpixmap.h>
#include <qtooltip.h>

#include "configobject.h"
#include "controlobject.h"
#include "wtracktable.h"
#include "wwidget.h"
#include "wknob.h"
#include "wpushbutton.h"
#include "wpushbuttoninc.h"
#include "wslider.h"
#include "wslidercomposed.h"
#include "wdisplay.h"
#include "wvumeter.h"
#include "wnumber.h"
#ifdef __VISUALS__
  #include "wvisual.h"
#endif

MixxxView::MixxxView(QWidget *parent, bool bVisuals, QString qSkinPath) : QWidget(parent, "Mixxx")
{
    // Path to image files
    WWidget::setPixmapPath(qSkinPath.append("/"));
    qDebug("skin %s",qSkinPath.latin1());

    // Read XML file
    QDomDocument skin("skin");
    QFile file(WWidget::getPath("skin.xml"));
    if (!file.open(IO_ReadOnly))
    {
        qFatal("Could not open skin definition file: %s",file.name().latin1());
    }
    if (!skin.setContent(&file))
    {
        qFatal("Error parsing skin definition file: %s",file.name().latin1());
    }
    file.close();
    QDomElement docElem = skin.documentElement();

#ifdef __WIN__
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::MemoryOptim);
#endif


    // Default values for visuals
    m_pVisualCh1 = 0;
    m_pVisualCh2 = 0;
    m_bZoom = false;
    
    // Load all widgets defined in the XML file
    QDomNode node = docElem.firstChild();
    while (!node.isNull())
    {
        if (node.isElement())
        {
            if (node.nodeName()=="PushButton")
            {
                WPushButton *p = new WPushButton(this);
                p->setup(node);
            }
            else if (node.nodeName()=="PushButtonInc")
            {
                WPushButtonInc *p = new WPushButtonInc(this);
                p->setup(node);
            }
            else if (node.nodeName()=="Knob")
            {
                WKnob *p = new WKnob(this);
                p->setup(node);
            }
            else if (node.nodeName()=="Display")
            {
                WDisplay *p = new WDisplay(this);
                p->setup(node);
            }
            else if (node.nodeName()=="Background")
            {
                QString filename = WWidget::selectNodeQString(node, "Path");
                QPixmap background(WWidget::getPath(filename));
                this->setPaletteBackgroundPixmap(background);
                this->setFixedSize(background.width(),background.height()+15);
                parent->setFixedSize(background.size());
                this->move(0,0);
            }
            else if (node.nodeName()=="SliderComposed")
            {
                WSliderComposed *p = new WSliderComposed(this);
                p->setup(node);

                // If rate slider...
                if (compareConfigKeys(node, "[Channel1],rate"))
                    m_pSliderRateCh1 = p;
                else if (compareConfigKeys(node, "[Channel2],rate"))
                    m_pSliderRateCh2 = p;
            }
            else if (node.nodeName()=="VuMeter")
            {
                WVuMeter *p = new WVuMeter(this);
                p->setup(node);
            }
            else if (node.nodeName()=="Visual")
            {
#ifdef __VISUALS__
                if (bVisuals)
                {
                    if (WWidget::selectNodeInt(node, "Channel")==1 && m_pVisualCh1==0)
                    {
                        m_pVisualCh1 = new WVisual(this, 0, 0);
                        if (m_pVisualCh1->isValid())
                        {
                            m_pVisualCh1->setup(node);
                            ControlObject::setWidget(m_pVisualCh1, ConfigKey("[Channel1]", "wheel"), true, Qt::LeftButton);
                        }
                        else
                        {
                            delete m_pVisualCh1;
                            m_pVisualCh1 = 0;
                        }
                    }
                    else if (WWidget::selectNodeInt(node, "Channel")==2 && m_pVisualCh1!=0 && m_pVisualCh2==0)
                    {
                        // Background color
                        m_pVisualCh2 = new WVisual(this, "", m_pVisualCh1);
                        if (m_pVisualCh2->isValid())
                        {
                            m_pVisualCh2->setup(node);
                            ControlObject::setWidget(m_pVisualCh2, ConfigKey("[Channel2]", "wheel"), true, Qt::LeftButton);
                        }
                        else
                        {
                            delete m_pVisualCh2;
                            m_pVisualCh2 = 0;
                        }
                    }

                    if (!WWidget::selectNode(node, "Zoom").isNull() && WWidget::selectNodeQString(node, "Zoom")=="true")
                        m_bZoom = true;
                }
#endif
            }
            else if (node.nodeName()=="Text")
            {
                QLabel *p = new QLabel(this);

                // Set position
                QString pos = WWidget::selectNodeQString(node, "Pos");
                int x = pos.left(pos.find(",")).toInt();
                int y = pos.mid(pos.find(",")+1).toInt();
                p->move(x,y);

                // Size
                QString size = WWidget::selectNodeQString(node, "Size");
                x = size.left(size.find(",")).toInt();
                y = size.mid(size.find(",")+1).toInt();
                p->setFixedSize(x,y);

                // Background color
                if (!WWidget::selectNode(node, "BgColor").isNull())
                {
                    QColor c;
                    c.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
                    p->setPaletteBackgroundColor(c);
                }
                
                // Foreground color
                if (!WWidget::selectNode(node, "FgColor").isNull())
                {
                    QColor c;
                    c.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
                    p->setPaletteForegroundColor(c);
                }

                // Associate pointers
                if (WWidget::selectNodeInt(node, "Channel")==1)
                    m_pTextCh1 = p;
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                    m_pTextCh2 = p;

            }
            else if (node.nodeName()=="TrackTable")
            {
                qDebug("Constructing TrackTable");
                m_pTrackTable = new WTrackTable(this);
                m_pTrackTable->setup(node);
            }

        }
        node = node.nextSibling();
    }

/*
    main = this;

    m_pBpmCh1 = new WNumber(main);
    m_pBpmCh1->setFixedSize(40,15);
    m_pBpmCh1->move(330,40);
    m_pBpmCh1->setNumDigits(6);

    m_pBpmCh2 = new WNumber(main);
    m_pBpmCh2->setFixedSize(40,15);
    m_pBpmCh2->move(900,40);
    m_pBpmCh2->setNumDigits(6);

    p->setWidget(m_pBpmCh1, ConfigKey("[Channel1]", "bpm"));
    p->setWidget(m_pBpmCh2, ConfigKey("[Channel2]", "bpm"));

    // EngineFlanger
    p->setWidget(flanger->DialDepth, ConfigKey("[Flanger]", "lfoDepth"));
    p->setWidget(flanger->DialDepth, ConfigKey("[Flanger]", "lfoDepth"));
    p->setWidget(flanger->DialPeriod, ConfigKey("[Flanger]", "lfoPeriod"));
    p->setWidget(flanger->PushButtonChA, ConfigKey("[Flanger]", "ch1"));
    p->setWidget(flanger->PushButtonChB, ConfigKey("[Flanger]", "ch2"));

*/

#ifdef __WIN__
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::NormalOptim);
#endif
}

MixxxView::~MixxxView()
{
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

