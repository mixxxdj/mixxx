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
    path = qSkinPath.append("/");
    qDebug("Skin path %s",path.latin1());
    WWidget::setPixmapPath(path);
    
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
                this->setFixedSize(background.size());
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
                        // Background color
                        if (!WWidget::selectNode(node, "BgColor").isNull())
                        {
                            QColor c;
                            c.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
                            m_pVisualCh1 = new WVisual(this, 0, 0, c);
                        }
                        else
                            m_pVisualCh1 = new WVisual(this);
                        
                        if (m_pVisualCh1->isValid())
                        {
                            // Set position
                            QString pos = WWidget::selectNodeQString(node, "Pos");
                            int x = pos.left(pos.find(",")).toInt();
                            int y = pos.mid(pos.find(",")+1).toInt();
                            m_pVisualCh1->move(x,y);

                            // Size
                            QString size = WWidget::selectNodeQString(node, "Size");
                            x = size.left(size.find(",")).toInt();
                            y = size.mid(size.find(",")+1).toInt();
                            m_pVisualCh1->setFixedSize(x,y);

                            m_pVisualCh1->show();
//                            ControlObject::setWidget(m_pVisualCh1, ConfigKey("[Channel1]", "wheel"), true, Qt::LeftButton);
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
                        if (!WWidget::selectNode(node, "BgColor").isNull())
                        {
                            QColor c;
                            c.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
                            m_pVisualCh2 = new WVisual(this, "", m_pVisualCh1, c);
                        }
                        else
                            m_pVisualCh2 = new WVisual(this,"",m_pVisualCh1);

                        if (m_pVisualCh2->isValid())
                        {
                            // Set position
                            QString pos = WWidget::selectNodeQString(node, "Pos");
                            int x = pos.left(pos.find(",")).toInt();
                            int y = pos.mid(pos.find(",")+1).toInt();
                            m_pVisualCh2->move(x,y);

                            // Size
                            QString size = WWidget::selectNodeQString(node, "Size");
                            x = size.left(size.find(",")).toInt();
                            y = size.mid(size.find(",")+1).toInt();
                            m_pVisualCh2->setFixedSize(x,y);

                            m_pVisualCh2->show();
//                            ControlObject::setWidget(m_pVisualCh2, ConfigKey("[Channel2]", "wheel"), true, Qt::LeftButton);
                        }
                        else
                        {
                            delete m_pVisualCh2;
                            m_pVisualCh2 = 0;
                        }
                    }
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
        
    m_pSliderPlayposCh1 = new WSliderComposed(main);
    m_pSliderPlayposCh1->setPixmaps(true, WWidget::getPath("sliders/playposslider.png"), WWidget::getPath("sliders/playposmarker.png"));
    m_pSliderPlayposCh1->setFixedSize(303,6);
    m_pSliderPlayposCh1->move(77,223);

    m_pSliderPlayposCh2 = new WSliderComposed(main);
    m_pSliderPlayposCh2->setPixmaps(true, WWidget::getPath("sliders/playposslider.png"), WWidget::getPath("sliders/playposmarker.png"));
    m_pSliderPlayposCh2->setFixedSize(303,6);
    m_pSliderPlayposCh2->move(644,223);

    m_pVUmeterCh1 = new WDisplay(main);
    m_pVUmeterCh1->setPositions(33);
    int i;
    for (i=0; i<10; ++i)
        m_pVUmeterCh1->setPixmap(i, WWidget::getPath(QString("vu-left/vu0%1.png").arg(i).latin1()));
    for (i=10; i<33; ++i)
        m_pVUmeterCh1->setPixmap(i, WWidget::getPath(QString("vu-left/vu%1.png").arg(i).latin1()));
    m_pVUmeterCh1->setFixedSize(15,105);
    m_pVUmeterCh1->move(485,99);

    m_pVUmeterCh2 = new WDisplay(main);
    m_pVUmeterCh2->setPositions(33);
    for (i=0; i<10; ++i)
        m_pVUmeterCh2->setPixmap(i, WWidget::getPath(QString("vu-right/vu0%1.png").arg(i).latin1()));
    for (i=10; i<33; ++i)
        m_pVUmeterCh2->setPixmap(i, WWidget::getPath(QString("vu-right/vu%1.png").arg(i).latin1()));
    m_pVUmeterCh2->setFixedSize(15,105);
    m_pVUmeterCh2->move(524,99);

    m_pBpmCh1 = new WNumber(main);
    m_pBpmCh1->setFixedSize(40,15);
    m_pBpmCh1->move(330,40);
    m_pBpmCh1->setNumDigits(6);

    m_pBpmCh2 = new WNumber(main);
    m_pBpmCh2->setFixedSize(40,15);
    m_pBpmCh2->move(900,40);
    m_pBpmCh2->setNumDigits(6);

    m_pEndOfTrackModeCh1 = new WPushButton(main);
    m_pEndOfTrackModeCh1->setStates(4);
    m_pEndOfTrackModeCh1->setPixmap(0, false, WWidget::getPath("buttons/endoftrackmode-stop.png"));
    m_pEndOfTrackModeCh1->setPixmap(0, true,  WWidget::getPath("buttons/endoftrackmode-stop.png"));
    m_pEndOfTrackModeCh1->setPixmap(1, false, WWidget::getPath("buttons/endoftrackmode-next.png"));
    m_pEndOfTrackModeCh1->setPixmap(1, true,  WWidget::getPath("buttons/endoftrackmode-next.png"));
    m_pEndOfTrackModeCh1->setPixmap(2, false, WWidget::getPath("buttons/endoftrackmode-loop.png"));
    m_pEndOfTrackModeCh1->setPixmap(2, true,  WWidget::getPath("buttons/endoftrackmode-loop.png"));
    m_pEndOfTrackModeCh1->setPixmap(3, false, WWidget::getPath("buttons/endoftrackmode-ping.png"));
    m_pEndOfTrackModeCh1->setPixmap(3, true,  WWidget::getPath("buttons/endoftrackmode-ping.png"));
    m_pEndOfTrackModeCh1->setFixedSize(30,10);
    m_pEndOfTrackModeCh1->move(330,70);

    m_pEndOfTrackModeCh2 = new WPushButton(main);
    m_pEndOfTrackModeCh2->setStates(4);
    m_pEndOfTrackModeCh2->setPixmap(0, false, WWidget::getPath("buttons/endoftrackmode-stop.png"));
    m_pEndOfTrackModeCh2->setPixmap(0, true,  WWidget::getPath("buttons/endoftrackmode-stop.png"));
    m_pEndOfTrackModeCh2->setPixmap(1, false, WWidget::getPath("buttons/endoftrackmode-next.png"));
    m_pEndOfTrackModeCh2->setPixmap(1, true,  WWidget::getPath("buttons/endoftrackmode-next.png"));
    m_pEndOfTrackModeCh2->setPixmap(2, false, WWidget::getPath("buttons/endoftrackmode-loop.png"));
    m_pEndOfTrackModeCh2->setPixmap(2, true,  WWidget::getPath("buttons/endoftrackmode-loop.png"));
    m_pEndOfTrackModeCh2->setPixmap(3, false, WWidget::getPath("buttons/endoftrackmode-ping.png"));
    m_pEndOfTrackModeCh2->setPixmap(3, true,  WWidget::getPath("buttons/endoftrackmode-ping.png"));
    m_pHeadCueCh1->setPixmapBackground(WWidget::getPath("buttons/cuecback1.png"));
    m_pEndOfTrackModeCh2->setFixedSize(30,10);
    m_pEndOfTrackModeCh2->move(900,70);

#ifdef __WIN__
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::NormalOptim);
#endif
*/
}

MixxxView::~MixxxView()
{
}

void MixxxView::assignWidgets(ControlObject *p)
{
    // EngineBuffer

//    p->setWidget(playcontrol1->PushButtonCueSet,ConfigKey("[Channel1]", "cue_set"));
//    p->setWidget(playcontrol2->PushButtonCueSet,ConfigKey("[Channel2]", "cue_set"));
//    p->setWidget(playcontrol1->PushButtonCueGoto, ConfigKey("[Channel1]", "cue_goto"));
//    p->setWidget(playcontrol2->PushButtonCueGoto, ConfigKey("[Channel2]", "cue_goto"));

    p->setWidget(m_pBpmCh1, ConfigKey("[Channel1]", "bpm"));
    p->setWidget(m_pBpmCh2, ConfigKey("[Channel2]", "bpm"));

    if (m_pVisualCh1)
    {
        p->setWidget(m_pVisualCh1, ConfigKey("[Channel1]", "wheel"), true, Qt::LeftButton);
        p->setWidget(m_pVisualCh2, ConfigKey("[Channel2]", "wheel"), true, Qt::LeftButton);
    }

    p->setWidget(m_pSliderRateCh1, ConfigKey("[Channel1]", "rate"), false);
    p->setWidget(m_pSliderRateCh2, ConfigKey("[Channel2]", "rate"), false);

    p->setWidget(m_pButtonRateUpCh1, ConfigKey("[Channel1]", "rate"), true, Qt::LeftButton, false);
    p->setWidget(m_pButtonRateUpCh1, ConfigKey("[Channel1]", "rate"), true, Qt::RightButton, false);
    p->setWidget(m_pButtonRateDownCh1, ConfigKey("[Channel1]", "rate"), true, Qt::LeftButton, false);
    p->setWidget(m_pButtonRateDownCh1, ConfigKey("[Channel1]", "rate"), true, Qt::RightButton, false);
    p->setWidget(m_pButtonRateUpCh2, ConfigKey("[Channel2]", "rate"), true, Qt::LeftButton, false);
    p->setWidget(m_pButtonRateUpCh2, ConfigKey("[Channel2]", "rate"), true, Qt::RightButton, false);
    p->setWidget(m_pButtonRateDownCh2, ConfigKey("[Channel2]", "rate"), true, Qt::LeftButton, false);
    p->setWidget(m_pButtonRateDownCh2, ConfigKey("[Channel2]", "rate"), true, Qt::RightButton, false);

    p->setWidget(m_pPlayCh1, ConfigKey("[Channel1]","play"), true, Qt::LeftButton);
    p->setWidget(m_pPlayCh2, ConfigKey("[Channel2]","play"), true, Qt::LeftButton);
    p->setWidget(m_pPlayCh1, ConfigKey("[Channel1]","cue_set"), true, Qt::RightButton);
    p->setWidget(m_pPlayCh2, ConfigKey("[Channel2]","cue_set"), true, Qt::RightButton);

    p->setWidget(m_pCueCh1, ConfigKey("[Channel1]","cue_preview"), true, Qt::LeftButton);
    p->setWidget(m_pCueCh2, ConfigKey("[Channel2]","cue_preview"), true, Qt::LeftButton);
    p->setWidget(m_pCueCh1, ConfigKey("[Channel1]","cue_preview"), false, Qt::LeftButton);
    p->setWidget(m_pCueCh2, ConfigKey("[Channel2]","cue_preview"), false, Qt::LeftButton);
    p->setWidget(m_pCueCh1, ConfigKey("[Channel1]","cue_goto"), true, Qt::RightButton);
    p->setWidget(m_pCueCh2, ConfigKey("[Channel2]","cue_goto"), true, Qt::RightButton);

    p->setWidget(m_pSliderPlayposCh1, ConfigKey("[Channel1]", "playposition"), false);
    p->setWidget(m_pSliderPlayposCh2, ConfigKey("[Channel2]", "playposition"), false);

    p->setWidget(m_pEndOfTrackModeCh1, ConfigKey("[Channel1]", "TrackEndMode"), false);
    p->setWidget(m_pEndOfTrackModeCh2, ConfigKey("[Channel2]", "TrackEndMode"), false);
                                            
    // EngineMaster
    p->setWidget(m_pHeadCueCh1, ConfigKey("[Channel1]", "pfl"));
    p->setWidget(m_pHeadCueCh2, ConfigKey("[Channel2]", "pfl"));

    p->setWidget(m_pSliderVolumeCh1, ConfigKey("[Channel1]", "volume"), false);
    p->setWidget(m_pSliderVolumeCh2, ConfigKey("[Channel2]", "volume"), false);

    // EnginePregain
    p->setWidget(m_pGainCh1, ConfigKey("[Channel1]", "pregain"));
    p->setWidget(m_pGainCh2, ConfigKey("[Channel2]", "pregain"));

    // EngineFilterBlock
    p->setWidget(m_pFilterLowCh1, ConfigKey("[Channel1]", "filterLow"));
    p->setWidget(m_pFilterLowCh2, ConfigKey("[Channel2]", "filterLow"));
    p->setWidget(m_pFilterMidCh1, ConfigKey("[Channel1]", "filterMid"));
    p->setWidget(m_pFilterMidCh2, ConfigKey("[Channel2]", "filterMid"));
    p->setWidget(m_pFilterHighCh1, ConfigKey("[Channel1]", "filterHigh"));
    p->setWidget(m_pFilterHighCh2, ConfigKey("[Channel2]", "filterHigh"));

    // Vu meter
    p->setWidget(m_pVUmeterCh1, ConfigKey("[Channel1]", "VUmeter"));
    p->setWidget(m_pVUmeterCh2, ConfigKey("[Channel2]", "VUmeter"));

/*
    // EngineFlanger
    p->setWidget(flanger->DialDepth, ConfigKey("[Flanger]", "lfoDepth"));
    p->setWidget(flanger->DialDepth, ConfigKey("[Flanger]", "lfoDepth"));
    p->setWidget(flanger->DialPeriod, ConfigKey("[Flanger]", "lfoPeriod"));
    p->setWidget(flanger->PushButtonChA, ConfigKey("[Flanger]", "ch1"));
    p->setWidget(flanger->PushButtonChB, ConfigKey("[Flanger]", "ch2"));

*/
    // EngineMaster
    p->setWidget(m_pSliderCrossfader, ConfigKey("[Master]", "crossfader"), false);
    p->setWidget(m_pVolume, ConfigKey("[Master]", "volume"));
    p->setWidget(m_pBalance, ConfigKey("[Master]", "balance"));
    p->setWidget(m_pHeadVolume, ConfigKey("[Master]", "headVolume"));
    p->setWidget(m_pHeadMix, ConfigKey("[Master]", "headMix"));
}

/*
const QString MixxxView::WWidget::getPath(QString location)
{
    QString l(location);
    return l.prepend(path);
}
*/

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

