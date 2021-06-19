#pragma once

#include <QHash>

#include "util/duration.h"

class ControlObject;

class SoftTakeover {
  public:
    // I would initialize it here but that's C++11 coolness. (Because it's a double.)
    static const double kDefaultTakeoverThreshold;

    SoftTakeover();
    bool ignore(ControlObject* control, double newParameter);
    void ignoreNext();
    void setThreshold(double threshold);

    struct TestAccess;

  private:
    // If a new value is received within this amount of time, jump to it
    // regardless. This allows quickly whipping controls to work while retaining
    // the benefits of soft-takeover for slower movements.  Setting this too
    // high will defeat the purpose of soft-takeover.
    static const mixxx::Duration kSubsequentValueOverrideTime;

    mixxx::Duration m_time;
    double m_prevParameter;
    double m_dThreshold;
};

struct SoftTakeover::TestAccess {
    static mixxx::Duration getTimeThreshold() {
        return kSubsequentValueOverrideTime;
    }
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
    // Ignore the next supplied parameter
    void ignoreNext(ControlObject* control);

  private:
    QHash<ControlObject*, SoftTakeover*> m_softTakeoverHash;
};
