#include "controllers/softtakeover.h"
#include "control/controlpotmeter.h"
#include "util/math.h"
#include "util/time.h"

// 3/128 units away from the current is enough to catch fast non-sequential moves
//  but not cause an audibly noticeable jump, determined experimentally with
//  slow-refresh controllers.
const double SoftTakeover::kDefaultTakeoverThreshold = 3.0 / 128;

const mixxx::Duration SoftTakeover::kSubsequentValueOverrideTime =
        mixxx::Duration::fromMillis(50);

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
    ControlPotmeter* cpo = qobject_cast<ControlPotmeter*>(control);
    if (cpo == nullptr) {
        // softtakecover works only for continuous ControlPotmeter based COs
        return;
    }

    // Initialize times
    if (!m_softTakeoverHash.contains(control)) {
        m_softTakeoverHash.insert(control, new SoftTakeover());
    }
}

void SoftTakeoverCtrl::disable(ControlObject* control) {
    if (control == nullptr) {
        return;
    }
    SoftTakeover* pSt = m_softTakeoverHash.take(control);
    if (pSt) {
        delete pSt;
    }
}

bool SoftTakeoverCtrl::ignore(ControlObject* control, double newParameter) {
    if (control == nullptr) {
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
    if (control == nullptr) {
        return;
    }

    SoftTakeover* pSt = m_softTakeoverHash.value(control);
    if (pSt == nullptr) {
        return;
    }

    pSt->ignoreNext();
}

SoftTakeover::SoftTakeover()
    : m_prevParameter(0),
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
     *      value of the ControlObject AND the new one arrives awhile after the
     *      previous one (regardless of what the previous value was)
     *  - new and previous values are on the same side of the current value of
     *      the ControlObject AND either:
     *      - the previous value is (also) far from the current CO value
     *          (regardless of the new value's arrival time)
     *      - the new value arrives awhile after the previous one
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

    mixxx::Duration currentTime = mixxx::Time::elapsed();
    // We will get a sudden jump if we don't ignore the first value.
    if (m_time == mixxx::Duration::fromMillis(0)) {
        ignore = true;
        // Change the stored time (but keep it far away from the current time)
        //  so this block doesn't run again.
        m_time = mixxx::Duration::fromMillis(1);
//         qDebug() << "SoftTakeover::ignore: ignoring the first value"
//                  << newParameter;
    } else if (currentTime - m_time > kSubsequentValueOverrideTime) {
        // don't ignore value if a previous one was not ignored in time
        const double currentParameter = control->getParameter();
        const double difference = currentParameter - newParameter;
        const double prevDiff = currentParameter - m_prevParameter;
        if ((prevDiff < 0 && difference < 0) ||
                (prevDiff > 0 && difference > 0)) {
            // On same side of the current parameter value
            if (fabs(difference) > m_dThreshold && fabs(prevDiff) > m_dThreshold) {
                // differences are above threshold
                ignore = true;
//                 qDebug() << "SoftTakeover::ignore: ignoring, not near"
//                          << newParameter << m_prevParameter << currentParameter;
            }
        }
    }
    if (!ignore) {
        // Update the time only if the value is not ignored. Replaces any
        // previous value for this control
        m_time = currentTime;
    }
    // Update the previous value every time
    m_prevParameter = newParameter;

    return ignore;
}

void SoftTakeover::ignoreNext() {
    m_time = mixxx::Duration::fromMillis(0);
}
