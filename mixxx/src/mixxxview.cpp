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
#include "dlgtracklist.h"
#include "dlgflanger.h"
#include "dlgmaster.h"
#include "dlgchannel.h"
#include "dlgplaycontrol.h"
#include "dlgcrossfader.h"
#include "dlgsplit.h"
#include "wtracktable.h"
#include "controlobject.h"
#include "wbulb.h"
#include "wknob.h"
#include "wpflbutton.h"
#include "wplaybutton.h"
#include "wplayposslider.h"
#include "wpushbutton.h"
#include "wslider.h"
#include "wtracktable.h"
#include "wvumeter.h"
#include "wwheel.h"

MixxxView::MixxxView(QWidget *parent) : QWidget(parent)
{
    // Sub widgets
    playcontrol1 = new DlgPlaycontrol(this);
    playcontrol2 = new DlgPlaycontrol(this); playcontrol2->layoutMirror();
    channel1 = new DlgChannel(this);
    channel2 = new DlgChannel(this); channel2->layoutMirror();
    master = new DlgMaster(this);
    crossfader = new DlgCrossfader(this);
    split = new DlgSplit(this);
    flanger = new DlgFlanger(this);
    tracklist = new DlgTracklist(this);

    // Layout management
    mainGrid = new QGridLayout(this,6,3); // A layout on a widget
    mainGrid->addWidget(playcontrol1,0,0);
    mainGrid->addWidget(channel1,0,1);
    mainGrid->addWidget(split,0,2);
    mainGrid->addWidget(channel2,0,3);
    mainGrid->addWidget(playcontrol2,0,4);
    mainGrid->addMultiCellWidget(master,0,1,5,5);
    mainGrid->addMultiCellWidget(flanger,2,2,5,5);
    mainGrid->addMultiCellWidget(crossfader,1,1,0,4);
    mainGrid->addMultiCellWidget(tracklist,2,2,0,4);

    //let the ratio between the widths of columns 0 and 1 be 2:3.
    mainGrid->setColStretch( 0, 240);
    mainGrid->setColStretch( 1,  70);
    mainGrid->setColStretch( 2,  16);
    mainGrid->setColStretch( 3,  70);
    mainGrid->setColStretch( 4, 240);
    mainGrid->setColStretch( 5,  90);

    mainGrid->setRowStretch( 0, 265);
    mainGrid->setRowStretch( 1,  43);
    mainGrid->setRowStretch( 2, 215);
}


MixxxView::~MixxxView()
{
}

void MixxxView::assignWidgets(ControlObject *p)
{
    // EngineBuffer
    p->setWidget(playcontrol1->PushButtonPlay, ConfigKey("[Channel1]","play"));
    p->setWidget(playcontrol2->PushButtonPlay, ConfigKey("[Channel2]","play"));
    p->setWidget(playcontrol1->PushButtonCueSet,ConfigKey("[Channel1]", "cue_set"));
    p->setWidget(playcontrol2->PushButtonCueSet,ConfigKey("[Channel2]", "cue_set"));
    p->setWidget(playcontrol1->PushButtonCueGoto, ConfigKey("[Channel1]", "cue_goto"));
    p->setWidget(playcontrol2->PushButtonCueGoto, ConfigKey("[Channel2]", "cue_goto"));
    p->setWidget(playcontrol1->SliderRate, ConfigKey("[Channel1]", "rate"));
    p->setWidget(playcontrol2->SliderRate, ConfigKey("[Channel2]", "rate"));
    p->setWidget(playcontrol1->WheelPlaycontrol, ConfigKey("[Channel1]", "wheel"));
    p->setWidget(playcontrol2->WheelPlaycontrol, ConfigKey("[Channel2]", "wheel"));
    p->setWidget(playcontrol1->SliderPosition, ConfigKey("[Channel1]", "playposition"));
    p->setWidget(playcontrol2->SliderPosition, ConfigKey("[Channel2]", "playposition"));

    // EngineMaster
    p->setWidget(channel1->CheckBoxPFL, ConfigKey("[Channel1]", "pfl"));
    p->setWidget(channel2->CheckBoxPFL, ConfigKey("[Channel2]", "pfl"));
    p->setWidget(channel1->SliderVolume, ConfigKey("[Channel1]", "volume"));
    p->setWidget(channel2->SliderVolume, ConfigKey("[Channel2]", "volume"));

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

    // EngineVUmeter
    p->setWidget(channel1->vumeter, ConfigKey("[Channel1]", "VUmeter"));
    p->setWidget(channel2->vumeter, ConfigKey("[Channel2]", "VUmeter"));

    // EngineFlanger
    p->setWidget(flanger->DialDepth, ConfigKey("[Flanger]", "lfoDepth"));
    p->setWidget(flanger->DialDepth, ConfigKey("[Flanger]", "lfoDepth"));
    p->setWidget(flanger->DialPeriod, ConfigKey("[Flanger]", "lfoPeriod"));
    p->setWidget(flanger->PushButtonChA, ConfigKey("[Flanger]", "ch1"));
    p->setWidget(flanger->PushButtonChB, ConfigKey("[Flanger]", "ch2"));

    // EngineMaster
    p->setWidget(crossfader->SliderCrossfader, ConfigKey("[Master]", "crossfader"));
    p->setWidget(master->KnobVolume, ConfigKey("[Master]", "volume"));
    p->setWidget(master->BulbClipping, ConfigKey("[Master]", "clipLed"));
    p->setWidget(master->vumeter, ConfigKey("[Master]", "VUmeter"));
    p->setWidget(master->KnobHeadVol, ConfigKey("[Master]", "headVolume"));
    p->setWidget(master->KnobHeadLR, ConfigKey("[Master]", "headMix"));
    
                                                                   
}                                                                 