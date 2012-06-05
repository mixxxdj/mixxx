/***************************************************************************
                           softtakeover.h  -  description
                           --------------
    begin                : Thu Mar 17 2011
    copyright            : (C) 2011 by Sean M. Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

#ifndef SOFTTAKEOVER_H
#define SOFTTAKEOVER_H

#include <QHash>

class ControlObject;

class SoftTakeover {
  public:
    // Enable soft-takeover for the given Control This does nothing on a Control
    // that's already enabled.
    void enable(ControlObject* control);
    // Disable soft-takeover for the given Control
    void disable(ControlObject* control);
    // Check to see if the new value for the Control should be ignored
    bool ignore(ControlObject* control, float newValue, bool midiVal = false);

    static uint currentTimeMsecs();

  private:
    // If a new value is received within this amount of time, jump to it
    // regardless. This allows quickly whipping controls to work while retaining
    // the benefits of soft-takeover for slower movements.  Setting this too
    // high will defeat the purpose of soft-takeover.
    static const uint SUBSEQUENT_VALUE_OVERRIDE_TIME_MILLIS = 50;

    QHash<ControlObject*, uint> m_times;
    QHash<ControlObject*, double> m_prevValues;
};

#endif
