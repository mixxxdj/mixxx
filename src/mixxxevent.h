/***************************************************************************
                          mixxxevent.h  -  enum of custom Qt events
                             -------------------
    begin                : Wed Feb 25 2009
    copyright            : (C) 2009 by nick@kousu.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/*class MixxxEvent : QEvent
{

  enum Type
  {
    Control = QEvent::User+1,
    Midi,
    Reader,
  };


}*/

//^ meh, take the short route (for now):
#define MIXXXEVENT_CONTROL ((QEvent::Type)(QEvent::User+1))
#define MIXXXEVENT_MIDI ((QEvent::Type)(QEvent::User+2))
#define MIXXXEVENT_READER ((QEvent::Type)(QEvent::User+3))

