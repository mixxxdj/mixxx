/***************************************************************************
                          softtakeover.cpp  -  description
                          ----------------
    begin                : Sat Mar 26 2011
    copyright            : (C) 2011 by Sean M. Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

#include <QDateTime>

#include "controllers/softtakeover.h"
#include "controlpotmeter.h"
#include "util/math.h"

// static
uint SoftTakeover::currentTimeMsecs() {
    QDateTime currDT = QDateTime::currentDateTime();

    // toMSecsSinceEpoch() is preferred since it's only one QDateTime call but
    //  it requires Qt 4.7. Use it instead when something more substantial
    //  elsewhere in Mixxx also requires Qt 4.7.
    //qint64 t = dt.toMSecsSinceEpoch();  // Requires Qt 4.7
    uint t = currDT.toTime_t()*1000+currDT.toString("zzz").toUInt();
    return t;
}

void SoftTakeover::enable(ControlObject* control) {
    if (control == NULL) {
        return;
    }
    // Initialize times
    if (!m_times.contains(control)) {
        m_times.insert(control, 0);
    }
    // Store current control value
    if (!m_prevValues.contains(control)) {
        m_prevValues.insert(control, control->get());
    }
}

void SoftTakeover::disable(ControlObject* control) {
    if (control == NULL) {
        return;
    }
    m_times.remove(control);
}

bool SoftTakeover::ignore(ControlObject* control, float newValue, bool midiVal) {
    if (control == NULL) {
        return false;
    }
    bool ignore = false;
    if (m_times.contains(control)) {
        // We only want to ignore the controller when all of the following are true:
        //  - its previous and new values are far away from and on the same side
        //      of the current value of the control
        //  - it's been awhile since the controller last affected this control

        // 3/128 units away from the current is enough to catch fast non-sequential moves
        //  but not cause an audibly noticeable jump.
        float threshold = 3.0f;

        if (!midiVal) {
            // HACK until we have Control 2.0. It can't come soon enough...
            ControlPotmeter* cpo = dynamic_cast<ControlPotmeter*>(control);
            if (cpo != NULL) {
                double maxValue = cpo->getMax();
                double minValue = cpo->getMin();
                double scaleFactor = maxValue - minValue;
                threshold = scaleFactor * (threshold / 128.0f);
            } else {
                // These defaults will effectively disable soft-takeover for this pass
                //  (causing the control to jump to the new value regardless)
                //  if there's a problem with the CO being NULL
                threshold =  10000000;  // Anything, just higher than any CO can go
            }
            // End hack
        }

        uint currentTime = currentTimeMsecs();
        uint time = m_times.value(control);
        // We will get a sudden jump if we don't ignore the first value.
        if (time == 0) {
            ignore = true;
            // Change the stored time (but keep it far away from the current time)
            //  so this block doesn't run again.
            m_times.insert(control, 1);
        } else if ((currentTime - time) > SUBSEQUENT_VALUE_OVERRIDE_TIME_MILLIS) {
            // don't ignore value if a previous one was not ignored in time
            double currentCoValue = midiVal ? control->getMidiParameter() : control->get();
            double difference = currentCoValue - newValue;
            double prevValue = m_prevValues.value(control, currentCoValue);
            double prevDiff = currentCoValue - prevValue;
            if ((prevDiff < 0 && difference < 0) ||
                    (prevDiff > 0 && difference > 0)) {
                // On same site (still on ignore site)
                if (fabs(difference) > threshold &&
                        fabs(prevDiff) > threshold) {
                    // difference is above threshold
                    ignore = true;
                }
            }
        }
        if (!ignore) {
            // Update the time only if the value is not ignored. Replaces any
            // previous value for this control
            m_times.insert(control, currentTime);
        }
        // Update the previous value every time
        m_prevValues.insert(control, newValue);
    }
    return ignore;
}
