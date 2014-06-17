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
#include "util/time.h"

SoftTakeoverCtrl::SoftTakeoverCtrl() {

}

SoftTakeoverCtrl::~SoftTakeoverCtrl() {
    QHashIterator<ControlObject*, SoftTakeover*> i(m_softTakeoverHash);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
}

void SoftTakeoverCtrl::enable(ControlObject* control) {
    ControlPotmeter* cpo = dynamic_cast<ControlPotmeter*>(control);
    if (cpo == NULL) {
        // softtakecover works only for continuous ControlPotmeter based COs
        return;
    }

    // Initialize times
    if (!m_softTakeoverHash.contains(control)) {
        m_softTakeoverHash.insert(control, new SoftTakeover());
    }
}

void SoftTakeoverCtrl::disable(ControlObject* control) {
    if (control == NULL) {
        return;
    }
    SoftTakeover* pSt = m_softTakeoverHash.take(control);
    if (pSt) {
        delete pSt;
    }
}

bool SoftTakeoverCtrl::ignore(ControlObject* control, double newMidiParameter) {
    if (control == NULL) {
        return false;
    }
    bool ignore = false;
    SoftTakeover* pSt = m_softTakeoverHash.value(control);
    if (pSt) {
        ignore = pSt->ignore(control, newMidiParameter);
    }
    return ignore;
}

SoftTakeover::SoftTakeover()
    : m_time(0),
      m_prevMidiParameter(0) {
}

bool SoftTakeover::ignore(ControlObject* control, double newMidiParameter) {
    bool ignore = false;
    // We only want to ignore the controller when all of the following are true:
    //  - its previous and new values are far away from and on the same side
    //      of the current value of the control
    //  - it's been awhile since the controller last affected this control

    // 3/128 units away from the current is enough to catch fast non-sequential moves
    //  but not cause an audibly noticeable jump.
    double threshold = 3.0f;

    uint currentTime = Time::elapsedMsecs();
    // We will get a sudden jump if we don't ignore the first value.
    if (m_time == 0) {
        ignore = true;
        // Change the stored time (but keep it far away from the current time)
        //  so this block doesn't run again.
        m_time = 1;
        qDebug() <<  "m_time == 0";
    } else if ((currentTime - m_time) > SUBSEQUENT_VALUE_OVERRIDE_TIME_MILLIS) {
        // don't ignore value if a previous one was not ignored in time
        double currentMidiParameter = control->getMidiParameter();
        double difference = currentMidiParameter - newMidiParameter;
        double prevDiff = currentMidiParameter - m_prevMidiParameter;
        if ((prevDiff < 0 && difference < 0) ||
                (prevDiff > 0 && difference > 0)) {
            // On same site (still on ignore site)
            if (fabs(difference) > threshold &&
                    fabs(prevDiff) > threshold) {
                // difference is above threshold
                ignore = true;
                qDebug() << currentMidiParameter << difference << prevDiff;
            }
        }
    }
    if (!ignore) {
        // Update the time only if the value is not ignored. Replaces any
        // previous value for this control
        m_time = currentTime;
    }
    qDebug() <<  m_prevMidiParameter << newMidiParameter << ignore << control->getKey().group << control->getKey().item;
    // Update the previous value every time
    m_prevMidiParameter = newMidiParameter;

    return ignore;
}

