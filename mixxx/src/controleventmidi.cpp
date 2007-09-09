/***************************************************************************
                          controleventmidi.cpp  -  description
                             -------------------
    begin                : Thu Feb 20 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "controleventmidi.h"

ControlEventMidi::ControlEventMidi(MidiCategory category, char channel, char control, char value) : QCustomEvent(10001), mcategory(category), mchannel(channel), mcontrol(control), mvalue(value)
{
};

ControlEventMidi::~ControlEventMidi()
{
};

MidiCategory ControlEventMidi::category() const
{
    return mcategory;
};

char ControlEventMidi::channel() const
{
    return mchannel;
};

char ControlEventMidi::control() const
{
    return mcontrol;
};

char ControlEventMidi::value() const
{
    return mvalue;
};
