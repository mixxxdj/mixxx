/***************************************************************************
                        mixxxutils.cpp - generic utilities useful in multiple
                        --------------   disparate places
                        
            copyright            : (C) 2010 by Sean Pappalardo
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

#include <QTime>

class MixxxUtils {
    public:
        /** Constructs a nicely formatted duration string from a given number of seconds */
        static inline QString secondsToMinutes(int totalSeconds) {
            if (totalSeconds < 0) return "?";
            
            QTime t = QTime().addSecs(totalSeconds);
            if (t.hour() >= 1)
                return t.toString("h:mm:ss");
            else
                return t.toString("m:ss");
            
            /*  // Manual way, for reference:
            QString result;
            
            int seconds = totalSeconds % 60;
            int mins = totalSeconds / 60;
            int hours = mins / 60;
            mins = mins - (hours*60);
            
            if (hours>0) result = QString("%1:%2:%3").arg(hours).arg(mins, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
            else result = QString("%1:%2").arg(mins).arg(seconds, 2, 10, QChar('0'));
            
            return result;
            */
        }
        
        /** Constructs a nicely formatted duration string from a given number of milliseconds */
        static inline QString millisecondsToMinutes(int totalMilliseconds) {
            if (totalMilliseconds < 0) return "?";
            
            QTime t = QTime().addMSecs(totalMilliseconds);
            if (t.hour() >= 1)
                return t.toString("h:mm:ss");
            else
                return t.toString("m:ss");
        }
};
        
        