/***************************************************************************
                          mixxxview.cpp  -  description
                             -------------------
    begin                : Mon Feb 18 09:48:17 CET 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#include "controlobject.h"
#include "wtracktable.h"
#include "wwidget.h"
#include "wknob.h"
#include "wpushbutton.h"
#include "wpushbuttoninc.h"
#include "wslider.h"
#include "wslidercomposed.h"
#include "wdisplay.h"
#include "wnumber.h"
#ifdef __VISUALS__
  #include "wvisual.h"
#endif

MixxxView::MixxxView(QWidget *parent, bool bVisuals) : QWidget(parent, "Mixxx")
{
    // Path to image files
    path = QDir::currentDirPath().append("/images/");
    qDebug("Image path %s",path.latin1());

#ifdef __WIN__
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::MemoryOptim);
#endif

    //
    // Construct main widget
    //
    main = this;
    main->setFixedSize(1024,768);
    QPixmap background(getPath("main.png"));
    main->setPaletteBackgroundPixmap(background);
    main->move(0,0);

    // Setup visuals
    m_pVisualCh1 = 0;
    m_pVisualCh2 = 0;
#ifdef __VISUALS__
    if (bVisuals)
    {
        m_pVisualCh1 = new WVisual(main);
        if (m_pVisualCh1->isValid())
        {
            m_pVisualCh2 = new WVisual(main,"",m_pVisualCh1);

            m_pVisualCh1->move(77,100);
            m_pVisualCh1->setFixedSize(303,120);
            m_pVisualCh2->move(644,100);
            m_pVisualCh2->setFixedSize(303,120);

            m_pVisualCh1->show();
            m_pVisualCh2->show();
        }
        else
        {
            delete m_pVisualCh1;
            m_pVisualCh1 = 0;
        }
    }
#endif

    m_pTrackTable = new WTrackTable(main);
    m_pTrackTable->move(76,490);
    m_pTrackTable->setFixedSize(870, 250);

    m_pTextCh1 = new QLabel(main);
    m_pTextCh1->setPaletteBackgroundColor(QColor(0,0,0));
    m_pTextCh1->setPaletteForegroundColor(QColor(0,254,0));
    m_pTextCh1->move(77,27);
    m_pTextCh1->setFixedSize(303, 70);
    
    m_pTextCh2 = new QLabel(main);
    m_pTextCh2->setPaletteBackgroundColor(QColor(0,0,0));
    m_pTextCh2->setPaletteForegroundColor(QColor(0,254,0));
    m_pTextCh2->move(644,27);
    m_pTextCh2->setFixedSize(303, 70);
    
/*    
    playcontrol1 = new DlgPlaycontrol(main);
    playcontrol1->move(76,26);
    playcontrol2 = new DlgPlaycontrol(main); playcontrol2->layoutMirror();
    playcontrol2->move(643,26);
*/
    m_pSliderCrossfader = new WSliderComposed(main);
    m_pSliderCrossfader->setPixmaps(true, getPath("sliders/cross.png"), getPath("sliders/knob3.png"));
    m_pSliderCrossfader->move(408,417);
    m_pSliderCrossfader->setFixedSize(208,37);
    
    m_pSliderVolumeCh1 = new WSliderComposed(main);
    m_pSliderVolumeCh1->setPixmaps(false, getPath("sliders/volleft.png"), getPath("sliders/knob1.png"));
    m_pSliderVolumeCh1->move(408,36);
    m_pSliderVolumeCh1->setFixedSize(27,207);

    m_pSliderVolumeCh2 = new WSliderComposed(main);
    m_pSliderVolumeCh2->setPixmaps(false, getPath("sliders/volright.png"), getPath("sliders/knob1.png"));
    m_pSliderVolumeCh2->move(589,36);
    m_pSliderVolumeCh2->setFixedSize(27,207);

    m_pSliderRateCh1 = new WSliderComposed(main);
    m_pSliderRateCh1->setPixmaps(false, getPath("sliders/pitchleft.png"), getPath("sliders/knob1.png"));
    m_pSliderRateCh1->move(73,255);
    m_pSliderRateCh1->setFixedSize(27,207);

    m_pSliderRateCh2 = new WSliderComposed(main);
    m_pSliderRateCh2->setPixmaps(false, getPath("sliders/pitchright.png"), getPath("sliders/knob1.png"));
    m_pSliderRateCh2->move(924,255);
    m_pSliderRateCh2->setFixedSize(27,207);

    m_pButtonRateUpCh1 = new WPushButtonInc(main);
    m_pButtonRateUpCh1->setPixmap(0, false, getPath("buttons/up0.png"));
    m_pButtonRateUpCh1->setPixmap(0, true, getPath("buttons/up3.png"));
    m_pButtonRateUpCh1->setFixedSize(28,88);
    m_pButtonRateUpCh1->move(104,246);
    m_pButtonRateUpCh1->setInc(0.03,0.01);
    
    m_pButtonRateDownCh1 = new WPushButtonInc(main);
    m_pButtonRateDownCh1->setPixmap(0, false, getPath("buttons/down0.png"));
    m_pButtonRateDownCh1->setPixmap(0, true, getPath("buttons/down3.png"));
    m_pButtonRateDownCh1->setFixedSize(28,88);
    m_pButtonRateDownCh1->move(104,383);
    m_pButtonRateDownCh1->setInc(-0.03,-0.01);

    m_pButtonRateUpCh2 = new WPushButtonInc(main);
    m_pButtonRateUpCh2->setPixmap(0, false, getPath("buttons/up0.png"));
    m_pButtonRateUpCh2->setPixmap(0, true, getPath("buttons/up3.png"));
    m_pButtonRateUpCh2->setFixedSize(28,88);
    m_pButtonRateUpCh2->move(892,246);
    m_pButtonRateUpCh2->setInc(0.03,0.01);

    m_pButtonRateDownCh2 = new WPushButtonInc(main);
    m_pButtonRateDownCh2->setPixmap(0, false, getPath("buttons/down0.png"));
    m_pButtonRateDownCh2->setPixmap(0, true, getPath("buttons/down3.png"));
    m_pButtonRateDownCh2->setFixedSize(28,88);
    m_pButtonRateDownCh2->move(892,383);
    m_pButtonRateDownCh2->setInc(-0.03,-0.01);
   
    m_pPlayCh1 = new WPushButton(main);
    m_pPlayCh1->setStates(2);
    m_pPlayCh1->setPixmap(0, false, getPath("buttons/play0.png"));
    m_pPlayCh1->setPixmap(0, true,  getPath("buttons/play1.png"));
    m_pPlayCh1->setPixmap(1, false, getPath("buttons/play3.png"));
    m_pPlayCh1->setPixmap(1, true,  getPath("buttons/play4.png"));
    m_pPlayCh1->setFixedSize(80,28);
    m_pPlayCh1->move(266,267);

    m_pPlayCh2 = new WPushButton(main);
    m_pPlayCh2->setStates(2);
    m_pPlayCh2->setPixmap(0, false, getPath("buttons/play0.png"));
    m_pPlayCh2->setPixmap(0, true,  getPath("buttons/play1.png"));
    m_pPlayCh2->setPixmap(1, false, getPath("buttons/play3.png"));
    m_pPlayCh2->setPixmap(1, true,  getPath("buttons/play4.png"));
    m_pPlayCh2->setFixedSize(80,28);
    m_pPlayCh2->move(678,267);

    m_pSliderPlayposCh1 = new WSliderComposed(main);
    m_pSliderPlayposCh1->setPixmaps(true, getPath("sliders/playposslider.png"), getPath("sliders/playposmarker.png"));
    m_pSliderPlayposCh1->setFixedSize(303,6);
    m_pSliderPlayposCh1->move(77,223);

    m_pSliderPlayposCh2 = new WSliderComposed(main);
    m_pSliderPlayposCh2->setPixmaps(true, getPath("sliders/playposslider.png"), getPath("sliders/playposmarker.png"));
    m_pSliderPlayposCh2->setFixedSize(303,6);
    m_pSliderPlayposCh2->move(644,223);

    m_pVUmeterCh1 = new WDisplay(main);
    m_pVUmeterCh1->setPositions(33);
    int i;
    for (i=0; i<10; ++i)
        m_pVUmeterCh1->setPixmap(i, getPath(QString("vu-left/vu0%1.png").arg(i).latin1()));
    for (i=10; i<33; ++i)
        m_pVUmeterCh1->setPixmap(i, getPath(QString("vu-left/vu%1.png").arg(i).latin1()));
    m_pVUmeterCh1->setFixedSize(15,105);
    m_pVUmeterCh1->move(485,99);

    m_pVUmeterCh2 = new WDisplay(main);
    m_pVUmeterCh2->setPositions(33);
    for (i=0; i<10; ++i)
        m_pVUmeterCh2->setPixmap(i, getPath(QString("vu-right/vu0%1.png").arg(i).latin1()));
    for (i=10; i<33; ++i)
        m_pVUmeterCh2->setPixmap(i, getPath(QString("vu-right/vu%1.png").arg(i).latin1()));
    m_pVUmeterCh2->setFixedSize(15,105);
    m_pVUmeterCh2->move(524,99);

    m_pVolume = new WKnob(main);
    m_pVolume->setPositions(10);
    for (i=0; i<10; ++i)
        m_pVolume->setPixmap(i, getPath(QString("knobs/knob%1.png").arg(i).latin1()));
    m_pVolume->setFixedSize(34,34);
    m_pVolume->move( 462,348);
    
    m_pHeadCueCh1 = new WPushButton(main);
    m_pHeadCueCh1->setStates(2);
    m_pHeadCueCh1->setPixmap(0, false, getPath("buttons/cuec0.png"));
    m_pHeadCueCh1->setPixmap(0, true,  getPath("buttons/cuec0.png"));
    m_pHeadCueCh1->setPixmap(1, false, getPath("buttons/cuec3.png"));
    m_pHeadCueCh1->setPixmap(1, true,  getPath("buttons/cuec3.png"));
    m_pHeadCueCh1->setFixedSize(34,13);
    m_pHeadCueCh1->move(477,218);

    m_pHeadCueCh2 = new WPushButton(main);
    m_pHeadCueCh2->setStates(2);
    m_pHeadCueCh2->setPixmap(0, false, getPath("buttons/cuec0.png"));
    m_pHeadCueCh2->setPixmap(0, true,  getPath("buttons/cuec0.png"));
    m_pHeadCueCh2->setPixmap(1, false, getPath("buttons/cuec3.png"));
    m_pHeadCueCh2->setPixmap(1, true,  getPath("buttons/cuec3.png"));
    m_pHeadCueCh2->setFixedSize(34,13);
    m_pHeadCueCh2->move(513,218);

    m_pCueCh1 = new WPushButton(main);
    m_pCueCh1->setStates(1);
    m_pCueCh1->setPixmap(0, false, getPath("buttons/cue0.png"));
    m_pCueCh1->setPixmap(0, true,  getPath("buttons/cue3.png"));
    m_pCueCh1->setFixedSize(55,28);
    m_pCueCh1->move(137,267);

    m_pCueCh2 = new WPushButton(main);
    m_pCueCh2->setStates(1);
    m_pCueCh2->setPixmap(0, false, getPath("buttons/cue0.png"));
    m_pCueCh2->setPixmap(0, true,  getPath("buttons/cue3.png"));
    m_pCueCh2->setFixedSize(55,28);
    m_pCueCh2->move(832,267);

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
    m_pEndOfTrackModeCh1->setPixmap(0, false, getPath("buttons/endoftrackmode-stop.png"));
    m_pEndOfTrackModeCh1->setPixmap(0, true,  getPath("buttons/endoftrackmode-stop.png"));
    m_pEndOfTrackModeCh1->setPixmap(1, false, getPath("buttons/endoftrackmode-next.png"));
    m_pEndOfTrackModeCh1->setPixmap(1, true,  getPath("buttons/endoftrackmode-next.png"));
    m_pEndOfTrackModeCh1->setPixmap(2, false, getPath("buttons/endoftrackmode-loop.png"));
    m_pEndOfTrackModeCh1->setPixmap(2, true,  getPath("buttons/endoftrackmode-loop.png"));
    m_pEndOfTrackModeCh1->setPixmap(3, false, getPath("buttons/endoftrackmode-ping.png"));
    m_pEndOfTrackModeCh1->setPixmap(3, true,  getPath("buttons/endoftrackmode-ping.png"));
    m_pEndOfTrackModeCh1->setFixedSize(30,10);
    m_pEndOfTrackModeCh1->move(330,70);

    m_pEndOfTrackModeCh2 = new WPushButton(main);
    m_pEndOfTrackModeCh2->setStates(4);
    m_pEndOfTrackModeCh2->setPixmap(0, false, getPath("buttons/endoftrackmode-stop.png"));
    m_pEndOfTrackModeCh2->setPixmap(0, true,  getPath("buttons/endoftrackmode-stop.png"));
    m_pEndOfTrackModeCh2->setPixmap(1, false, getPath("buttons/endoftrackmode-next.png"));
    m_pEndOfTrackModeCh2->setPixmap(1, true,  getPath("buttons/endoftrackmode-next.png"));
    m_pEndOfTrackModeCh2->setPixmap(2, false, getPath("buttons/endoftrackmode-loop.png"));
    m_pEndOfTrackModeCh2->setPixmap(2, true,  getPath("buttons/endoftrackmode-loop.png"));
    m_pEndOfTrackModeCh2->setPixmap(3, false, getPath("buttons/endoftrackmode-ping.png"));
    m_pEndOfTrackModeCh2->setPixmap(3, true,  getPath("buttons/endoftrackmode-ping.png"));
    m_pEndOfTrackModeCh2->setFixedSize(30,10);
    m_pEndOfTrackModeCh2->move(900,70);

#ifdef __WIN__
    // QPixmap fix needed on Windows 9x
    QPixmap::setDefaultOptimization(QPixmap::NormalOptim);
#endif

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

    p->setWidget(m_pEndOfTrackModeCh1, ConfigKey("[Channel1]", "EndOfTrackMode"), false);
    p->setWidget(m_pEndOfTrackModeCh2, ConfigKey("[Channel2]", "EndOfTrackMode"), false);
    
    // EngineMaster
    p->setWidget(m_pHeadCueCh1, ConfigKey("[Channel1]", "pfl"));
    p->setWidget(m_pHeadCueCh2, ConfigKey("[Channel2]", "pfl"));

    p->setWidget(m_pSliderVolumeCh1, ConfigKey("[Channel1]", "volume"), false);
    p->setWidget(m_pSliderVolumeCh2, ConfigKey("[Channel2]", "volume"), false);
/*
    // EnginePregain
    p->setWidget(channel1->DialGain, ConfigKey("[Channel1]", "pregain"));
    p->setWidget(channel2->DialGain, ConfigKey("[Channel2]", "pregain"));

    // EngineFilterBlock
    p->setWidget(channel1->DialFilterLow, ConfigKey("[Channel1]", "filterLow"));
    p->setWidget(channel2->DialFilterLow, ConfigKey("[Channel2]", "filterLow"));
    p->setWidget(channel1->DialFilterMiddle, ConfigKey("[Channel1]", "filterMid"));
    p->setWidget(channel2->DialFilterMiddle, ConfigKey("[Channel2]", "filterMid"));
    p->setWidget(channel1->DialFilterHigh, ConfigKey("[Channel1]", "filterHigh"));
    p->setWidget(channel2->DialFilterHigh, ConfigKey("[Channel2]", "filterHigh"));

    // EngineClipping
    p->setWidget(channel1->BulbClipping, ConfigKey("[Channel1]", "clipLed"));
    p->setWidget(channel2->BulbClipping, ConfigKey("[Channel2]", "clipLed"));

*/

    // EngineVUmeter
    p->setWidget(m_pVUmeterCh1, ConfigKey("[Channel1]", "VUmeter"));
    p->setWidget(m_pVUmeterCh2, ConfigKey("[Channel2]", "VUmeter"));

/*
    // EngineFlanger
    p->setWidget(flanger->DialDepth, ConfigKey("[Flanger]", "lfoDepth"));
    p->setWidget(flanger->DialDepth, ConfigKey("[Flanger]", "lfoDepth"));
    p->setWidget(flanger->DialPeriod, ConfigKey("[Flanger]", "lfoPeriod"));
    p->setWidget(flanger->PushButtonChA, ConfigKey("[Flanger]", "ch1"));
    p->setWidget(flanger->PushButtonChB, ConfigKey("[Flanger]", "ch2"));

    // EngineMaster
*/
    p->setWidget(m_pSliderCrossfader, ConfigKey("[Master]", "crossfader"), false);
/*
    p->setWidget(master->KnobVolume, ConfigKey("[Master]", "volume"));
    p->setWidget(master->BulbClipping, ConfigKey("[Master]", "clipLed"));
    p->setWidget(master->vumeter, ConfigKey("[Master]", "VUmeter"));
    p->setWidget(master->KnobHeadVol, ConfigKey("[Master]", "headVolume"));
    p->setWidget(master->KnobHeadLR, ConfigKey("[Master]", "headMix"));
*/    
                                                                   
}

const QString MixxxView::getPath(QString location)
{
    QString l(location);
    return l.prepend(path);
}
