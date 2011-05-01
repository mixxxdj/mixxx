/***************************************************************************
                           softtakeover.h  -  description
                           --------------
    begin                : Thu Mar 17 2011
    copyright            : (C) 2011 by Sean M. Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef SOFTTAKEOVER_H
#define SOFTTAKEOVER_H

#include "mixxxcontrol.h"

class SoftTakeover {

    public:
        /** Enable soft-takeover for the given Control
            It's okay to call this on a Control that's already enabled. */
        void enable(MixxxControl control);
        void enable(QString group, QString name);
        /** Disable soft-takeover for the given Control */
        void disable(MixxxControl control);
        void disable(QString group, QString name);
        /** Check to see if the new value for the Control should be ignored */
        bool ignore(MixxxControl control, float newValue, bool midiVal = false);
        /** For legacy Controls */
        bool ignore(QString group, QString name, float newValue);
    
    private:
        /** If a new value is received within this amount of time,
            jump to it regardless. This allows quickly whipping controls to work
            while retaining the benefits of soft-takeover for slower movements.
            
            Setting this too high will defeat the purpose of soft-takeover.*/        
        static const uint SUBSEQUENT_VALUE_OVERRIDE_TIME = 50;   // Milliseconds
        //qint64 currentTimeMsecs();
        //QHash<MixxxControl,qint64> m_times;
        uint currentTimeMsecs();
        QHash<MixxxControl,uint> m_times;
};

#endif