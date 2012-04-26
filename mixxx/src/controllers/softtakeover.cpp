/***************************************************************************
                          softtakeover.cpp  -  description
                          ----------------
    begin                : Sat Mar 26 2011
    copyright            : (C) 2011 by Sean M. Pappalardo
    email                : spappalardo@mixxx.org
 ***************************************************************************/

#include <math.h>  // for fabs()
#include <qdatetime.h>

#include "controllers/softtakeover.h"
#include "controlpotmeter.h"

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
    // Store current time
    if (!m_times.contains(control)) {
        m_times.insert(control, currentTimeMsecs());
    }
    // Store current MixxxControl value
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
        //      of the current value of the MixxxControl
        //  - it's been awhile since the controller last affected this MixxxControl

        // 3/128 units away from the current is enough to catch fast non-sequential moves
        //  but not cause an audially noticeable jump.
        float threshold = 3.0f;

        if (!midiVal) {
            // These defaults will effectively disable soft-takeover for this pass
            //  (causing the control to jump to the new value regardless)
            //  if there's a problem with the below CO being NULL
            double maxValue=10000000;   // Anything, just higher than any CO can go
            double minValue=0;

            // HACK until we have Control 2.0. It can't come soon enough...
            ControlPotmeter* cpo = dynamic_cast<ControlPotmeter*>(control);
            if (cpo != NULL) {
                maxValue = cpo->getMax();
                minValue = cpo->getMin();
            }
            // End hack

            double scaleFactor = maxValue-minValue;
            threshold = scaleFactor*(threshold/128.0f);
        }

        double currentValue = midiVal ? control->GetMidiValue() : control->get();
        double difference = currentValue - newValue;
        double prevDiff = 0;
        bool sameSide = false;
        if (m_prevValues.contains(control)) {
            double prevValue = m_prevValues.value(control);
            prevDiff = currentValue - prevValue;
            if ((prevDiff < 0 && difference < 0) ||
                (prevDiff > 0 && difference > 0)) {
                sameSide = true;
            }
        }

        uint currentTime = currentTimeMsecs();
        if (fabs(difference)>threshold &&
            fabs(prevDiff)>threshold && sameSide &&
            (currentTime - m_times.value(control)) > SUBSEQUENT_VALUE_OVERRIDE_TIME_MILLIS) {
            ignore = true;
        }
        if (!ignore) {
            // Update the time only if the value is not ignored. Replaces any
            // previous value for this MixxxControl
            m_times.insert(control, currentTime);
        }
        // Update the previous value every time
        m_prevValues.insert(control, newValue);
    }
    return ignore;
}
