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

// 3/128 units away from the current is enough to catch fast non-sequential moves
//  but not cause an audibly noticeable jump.


class SoftTakeover {
  public:
    static const double kDefaultTakeoverThreshold;

    SoftTakeover();
    bool ignore(ControlObject* control, double newParameter);
    void ignoreNext();
    void setThreshold(double threshold);

  private:
    // If a new value is received within this amount of time, jump to it
    // regardless. This allows quickly whipping controls to work while retaining
    // the benefits of soft-takeover for slower movements.  Setting this too
    // high will defeat the purpose of soft-takeover.
    static const uint SUBSEQUENT_VALUE_OVERRIDE_TIME_MILLIS = 50;

    uint m_time;
    double m_prevParameter;
    double m_dThreshold;
};

class SoftTakeoverCtrl {
  public:
    SoftTakeoverCtrl();
    ~SoftTakeoverCtrl();

    // Enable soft-takeover for the given Control.
    // This does nothing on a control that already has soft-takeover enabled.
    void enable(ControlObject* control);
    // Disable soft-takeover for the given Control
    void disable(ControlObject* control);
    // Check to see if the new value for the Control should be ignored
    bool ignore(ControlObject* control, double newMidiParameter);

  private:
    QHash<ControlObject*, SoftTakeover*> m_softTakeoverHash;
};

#endif
