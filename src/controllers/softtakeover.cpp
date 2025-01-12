#include "controllers/softtakeover.h"

#include <cmath>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"

using namespace std::chrono_literals;

void SoftTakeoverCtrl::enable(gsl::not_null<ControlPotmeter*> pControl) {
    // explicitly not in the header to avoid adding dependency on ControlPotmeter
    m_softTakeoverHash.try_emplace(static_cast<ControlObject*>(pControl.get()));
}

bool SoftTakeover::willIgnore(const ControlObject& control,
        double newParameter,
        ClockT::time_point currentTime) const {
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

    if (m_time == kFirstValueTime) {
        return true;
    }
    // don't ignore value if a previous one was not ignored in time
    if (currentTime < m_time + kSubsequentValueOverrideTime) {
        return false;
    }
    const double currentParameter = control.getParameter();
    const double difference = currentParameter - newParameter;
    const double prevDiff = currentParameter - m_prevParameter;
    // Don't ignore if opposite side of the current parameter value
    if (std::signbit(prevDiff) != std::signbit(difference)) {
        return false;
    }
    // On same side of the current parameter value
    if (fabs(difference) <= m_dThreshold || fabs(prevDiff) <= m_dThreshold) {
        // differences are below threshold
        return false;
    }
    return true;
}

bool SoftTakeover::ignore(const ControlObject& control, double newParameter) {
    bool ignore = false;

    auto currentTime = ClockT::now();
    // We will get a sudden jump if we don't ignore the first value.
    if (m_time == kFirstValueTime) {
        ignore = true;
        // Change the stored time (but keep it far away from the current time)
        //  so this block doesn't run again.
        m_time = ClockT::time_point(kFirstValueTime + 1ms);
    } else {
        ignore = willIgnore(control, newParameter, currentTime);
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
