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
    static inline QString secondsToMinutes(int totalSeconds, bool showMillis=false) {
        if (totalSeconds < 0)
            return "?";
        return millisecondsToMinutes(totalSeconds*1000, showMillis);
    }

    /** Constructs a nicely formatted duration string from a given number of milliseconds */
    static inline QString millisecondsToMinutes(int totalMilliseconds, bool showMillis=false) {
        if (totalMilliseconds < 0)
            return "?";
        QTime t = QTime().addMSecs(totalMilliseconds);
        QString formatString = (t.hour() >= 1) ? "hh:mm:ss" : "mm:ss";
        if (showMillis)
            formatString = formatString.append(".zzz");
        QString timeString = t.toString(formatString);
        // The format string gives us one extra digit of millisecond precision than
        // we care about. Slice it off.
        if (showMillis)
            timeString = timeString.left(timeString.length() - 1);
        return timeString;
    }
};
