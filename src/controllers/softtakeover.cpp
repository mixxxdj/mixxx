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

// 3/128 units away from the current is enough to catch fast non-sequential moves
//  but not cause an audibly noticeable jump, determined experimentally with
//  slow-refresh controllers.
const double SoftTakeover::kDefaultTakeoverThreshold = 3.0 / 128;


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

bool SoftTakeoverCtrl::ignore(ControlObject* control, double newParameter) {
    if (control == NULL) {
        return false;
    }
    bool ignore = false;
    SoftTakeover* pSt = m_softTakeoverHash.value(control);
    if (pSt) {
        ignore = pSt->ignore(control, newParameter);
    }
    return ignore;
}

void SoftTakeoverCtrl::ignoreNext(ControlObject* control) {
    if (control != NULL) {
        SoftTakeover* pSt = m_softTakeoverHash.value(control);
        if (pSt) {
            pSt->ignoreNext();
        }
    }
}

SoftTakeover::SoftTakeover()
    : m_time(-1),
      m_prevParameter(-1),
      m_dThreshold(kDefaultTakeoverThreshold) {
}

void SoftTakeover::setThreshold(double threshold) {
    m_dThreshold = threshold;
}

bool SoftTakeover::ignore(ControlObject* control, double newParameter) {
    bool ignore = false;
    /* 
     * We only want to ignore the controller when:
     * - its new value is far away from the current value of the ControlObject
     * AND either of the following:
     *  - its new and previous values are on the opposite side of the current
     *      value of the ControlObject AND the new one arrives long enough after the
     *      previous one (regardless of what the previous value was)
     *  - new and previous values are on the same side of the current value of
     *      the ControlObject AND either:
     *      - the previous value is (also) far from the current CO value
     *          (regardless of the new value's arrival time)
     *      - the new value arrives long enough after the previous one
     *          (regardless of what the previous value was)
     * 
     * Sheesh, this is much easier to show in a truth table!
     * 
     * Sides    prev distance   new distance    new value arrives   Ignore
     * opposite close           far             later               TRUE
     * opposite far             far             later               TRUE
     * same     close           far             later               TRUE
     * same     far             far             soon                TRUE
     * same     far             far             later               TRUE
     * 
     *      Don't ignore in every other case.
     */

    qint64 currentTime = Time::elapsedMsecs();
    // We will get a sudden jump if we don't ignore the first value.
    if (m_time == -1) {
        ignore = true;
        // Change the stored time (but keep it far away from the current time)
        //  so this block doesn't run again.
        m_time = -2;    // Tests like to use 0
        //qDebug() << "ignoring the first value" << newParameter;
    } else {
        const double currentParameter = control->getParameter();
        const double difference = currentParameter - newParameter;
        if (m_prevParameter != -1 && fabs(difference) > m_dThreshold) {
            // New parameter is far away from current
            const double prevDiff = currentParameter - m_prevParameter;
            if (prevDiff * difference <= 0) {
                // Opposite sides
                if ((currentTime - m_time) > kSubsequentValueOverrideTimeMsecs) {
                    // New parameter arrived long enough after the last one that
                    //  took effect
                    ignore = true;
                    
                    // Except if the previous value was the same as the current,
                    //  we need to process it anyway to allow really fast movements
                    if (prevDiff == 0) { ignore = false; }
                    //  This causes bug #1520798 where changing control mappings
                    //  on-the-fly causes the first message to be processed 
                    //  regardless when those controls reconnect.
                    //  The only mitigation is for those controls to
                    //  call SoftTakeover::ignoreNext() on reconnection
                    //      - Sean 12/2015
                    
                    //if (ignore) {
                    //qDebug() << "ignoring new parameter" << newParameter
                    //   << "is further than" << m_dThreshold << "from current" 
                    //   << currentParameter
                    //   << "on opposite side of" << m_prevParameter << "and arrived"
                    //   << (currentTime - m_time) << "msecs after it.";
                    //}
                }
            } else {
                // Same side
                if (fabs(prevDiff) > m_dThreshold) {
                    // Previous parameter is far away from current
                    ignore = true;
                    //qDebug() << "ignoring new parameter" << newParameter
                    //   << "is further than" << m_dThreshold << "from current" 
                    //   << currentParameter
                    //   << "on same side of" << m_prevParameter 
                    //   << "which is also far from current";
                } else 
                    if ((currentTime - m_time) > kSubsequentValueOverrideTimeMsecs) {
                    // New parameter arrived long enough after the last one that
                    //  took effect
                    ignore = true;
                    //qDebug() << "ignoring new parameter" << newParameter
                    //   << "is further than" << m_dThreshold << "from current" 
                    //   << currentParameter
                    //   << "on same side of" << m_prevParameter << "and arrived"
                    //   << (currentTime - m_time) << "msecs after it.";
                }
            }
        }
    }


    if (!ignore) {
        // Update the time only if the value is not ignored. Replaces any
        // previous value for this ControlObject
        //qDebug() << "Not ignored" << newParameter << ", old time" << m_time 
        //         << "storing time" << currentTime;
        m_time = currentTime;
    }
    // Update the previous value every time
    m_prevParameter = newParameter;

    return ignore;
}

void SoftTakeover::ignoreNext() {
    m_time = -1;
}
