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
#include <qevent.h>

#include "wtreelist.h"
#include "configobject.h"
#include "controlobject.h"
#include "wtracktable.h"
#include "wwidget.h"
#include "wknob.h"
#include "wpushbutton.h"
#include "wslider.h"
#include "wslidercomposed.h"
#include "wdisplay.h"
#include "wvumeter.h"
#include "wnumber.h"
#include "wnumberpos.h"
#include "wnumberbpm.h"
#include "wnumberrate.h"
#include "wvisualwaveform.h"
#include "wvisualsimple.h"

MixxxView::MixxxView(QWidget *parent, bool bVisualsWaveform, QString qSkinPath, ConfigObject<ConfigValue> *) : QWidget(parent, "Mixxx")
{
    // Path to image files
    WWidget::setPixmapPath(qSkinPath.append("/"));

    m_qWidgetList.setAutoDelete(true);

    //qDebug("skin %s",qSkinPath.latin1());

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
    m_bVisualWaveform = false;

    m_pNumberPosCh1 = 0;
    m_pNumberPosCh2 = 0;
    
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
                m_qWidgetList.append(p);
            }
            else if (node.nodeName()=="Knob")
            {
                WKnob *p = new WKnob(this);
                p->setup(node);
                m_qWidgetList.append(p);
            }
            else if (node.nodeName()=="Number")
            {
                WNumber *p = new WNumber(this);
                p->setup(node);
                m_qWidgetList.append(p);
            }
            else if (node.nodeName()=="NumberBpm")
            {
                if (WWidget::selectNodeInt(node, "Channel")==1)
                {
                    WNumberBpm *p = new WNumberBpm("[Channel1]", this);
                    p->setup(node);
                    m_qWidgetList.append(p);
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                {
                    WNumberBpm *p = new WNumberBpm("[Channel2]", this);
                    p->setup(node);
                    m_qWidgetList.append(p);
                }
            }
            else if (node.nodeName()=="NumberPos")
            {
                if (WWidget::selectNodeInt(node, "Channel")==1 && m_pNumberPosCh1==0)
                {
                    m_pNumberPosCh1 = new WNumberPos("[Channel1]", this);
                    m_pNumberPosCh1->setup(node);
                    m_qWidgetList.append(m_pNumberPosCh1);
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2 && m_pNumberPosCh2==0)
                {
                    m_pNumberPosCh2 = new WNumberPos("[Channel2]", this);
                    m_pNumberPosCh2->setup(node);
                    m_qWidgetList.append(m_pNumberPosCh2);
                }
            }
            else if (node.nodeName()=="NumberRate")
            {
                if (WWidget::selectNodeInt(node, "Channel")==1)
                {
                    WNumberRate *p = new WNumberRate("[Channel1]", this);
                    p->setup(node);
                    m_qWidgetList.append(p);
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                {
                    WNumberRate *p = new WNumberRate("[Channel2]", this);
                    p->setup(node);
                    m_qWidgetList.append(p);
                }
            }
            else if (node.nodeName()=="Display")
            {
                WDisplay *p = new WDisplay(this);
                p->setup(node);
                m_qWidgetList.append(p);
            }
            else if (node.nodeName()=="Background")
            {
                QString filename = WWidget::selectNodeQString(node, "Path");
                QPixmap background(WWidget::getPath(filename));
                this->setPaletteBackgroundPixmap(background);
                this->setFixedSize(background.width(),background.height());
                parent->setFixedSize(background.size());
                this->move(0,0);
            }
            else if (node.nodeName()=="SliderComposed")
            {
                WSliderComposed *p = new WSliderComposed(this);
                p->setup(node);
                m_qWidgetList.append(p);

                // If rate slider...
                if (compareConfigKeys(node, "[Channel1],rate"))
                    m_pSliderRateCh1 = p;
                else if (compareConfigKeys(node, "[Channel2],rate"))
                    m_pSliderRateCh2 = p;
            }
            else if (node.nodeName()=="VuMeter")
            {
                WVuMeter *p = new WVuMeter(this);
                m_qWidgetList.append(p);
                p->setup(node);
            }
            else if (node.nodeName()=="Visual")
            {
                if (WWidget::selectNodeInt(node, "Channel")==1 && m_pVisualCh1==0)
                {
                    if (bVisualsWaveform)
                    {
                        m_pVisualCh1 = new WVisualWaveform(this, 0, 0);
                        if (((WVisualWaveform *)m_pVisualCh1)->isValid())
                        {
                            ((WVisualWaveform *)m_pVisualCh1)->setup(node);
                            m_qWidgetList.append(m_pVisualCh1);
                            m_bVisualWaveform = true;
                        }
                        else
                        {
                            m_bVisualWaveform = false;
                            delete m_pVisualCh1;
                        }
                    }
                    if (!m_bVisualWaveform)
                    {
                        m_pVisualCh1 = new WVisualSimple(this, 0);
                        ((WVisualSimple *)m_pVisualCh1)->setup(node);
                        m_qWidgetList.append(m_pVisualCh1);
                    }
                    ControlObject::setWidget((QWidget *)m_pVisualCh1, ConfigKey("[Channel1]", "wheel"), true, Qt::LeftButton);
                }
                else if (WWidget::selectNodeInt(node, "Channel")==2 && m_pVisualCh1!=0 && m_pVisualCh2==0)
                {
                    if (bVisualsWaveform)
                    {
                        m_pVisualCh2 = new WVisualWaveform(this, "", (QGLWidget *)m_pVisualCh1);
                        if (((WVisualWaveform *)m_pVisualCh2)->isValid())
                        {
                            ((WVisualWaveform *)m_pVisualCh2)->setup(node);
                            m_bVisualWaveform = true;
                            m_qWidgetList.append(m_pVisualCh2);
                        }
                        else
                        {
                            m_bVisualWaveform = false;
                            delete m_pVisualCh2;
                        }
                    }
                    if (!m_bVisualWaveform)
                    {
                        m_pVisualCh2 = new WVisualSimple(this, 0);
                        ((WVisualSimple *)m_pVisualCh2)->setup(node);
                        m_qWidgetList.append(m_pVisualCh2);
                    }
                    ControlObject::setWidget((QWidget *)m_pVisualCh2, ConfigKey("[Channel2]", "wheel"), true, Qt::LeftButton);
                }

                if (!WWidget::selectNode(node, "Zoom").isNull() && WWidget::selectNodeQString(node, "Zoom")=="true")
                    m_bZoom = true;
            }
            else if (node.nodeName()=="Text")
            {
                QLabel *p = new QLabel(this);
                m_qWidgetList.append(p);

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

                // Alignment
                if (!WWidget::selectNode(node, "Align").isNull() && WWidget::selectNodeQString(node, "Align")=="right")
                    p->setAlignment(Qt::AlignRight);

                // Associate pointers
                if (WWidget::selectNodeInt(node, "Channel")==1)
                    m_pTextCh1 = p;
                else if (WWidget::selectNodeInt(node, "Channel")==2)
                    m_pTextCh2 = p;

            }
            else if (node.nodeName()=="TrackTable")
            {
                //qDebug("Constructing TrackTable");
                m_pTrackTable = new WTrackTable(this);
                m_pTrackTable->setup(node);
                m_qWidgetList.append(m_pTrackTable);
            }
			else if (node.nodeName()=="TreeList")
			{
				qDebug("Constructing TreeList");
				m_pTreeList = new WTreeList(this,tr("TreeList"));
				m_pTreeList->setup(node);
				//Here the TreeList view should be instantiated
			}

        }
        node = node.nextSibling();
    }

#ifdef __WIN__
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::NormalOptim);
#endif

}

MixxxView::~MixxxView()
{
    m_qWidgetList.clear();
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
