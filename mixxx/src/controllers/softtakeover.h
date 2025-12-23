#pragma once

#include <chrono>
#include <gsl/pointers>
#include <unordered_map>

#include "util/assert.h"
#include "util/time.h"

class ControlObject;
class ControlPotmeter;

class SoftTakeover {
  public:
    using ClockT = mixxx::Time;
    // 3/128 units away from the current is enough to catch fast non-sequential moves
    //  but not cause an audibly noticeable jump, determined experimentally with
    //  slow-refresh controllers.
    // TODO (XXX): Expose this to the controller mapping environment?
    static constexpr double kDefaultTakeoverThreshold = 3.0 / 128;

    SoftTakeover() = default;
    bool ignore(const ControlObject& control, double newParameter);
    bool willIgnore(const ControlObject& control,
            double newParameter,
            ClockT::time_point currentTime) const;
    void ignoreNext() {
        m_time = kFirstValueTime;
    }
    void setThreshold(double threshold) {
        m_dThreshold = threshold;
    }

    // TODO (XXX): find a better testing solution than this TestAccess
    // front-door coupled to `mixxx::Time`.
    struct TestAccess {
        static constexpr ClockT::duration getTimeThreshold() {
            return kSubsequentValueOverrideTime;
        }
        template<class Rep = ClockT::rep,
                class Period = ClockT::period>
        static void advanceTimePastThreshold(
                std::chrono::duration<Rep, Period> offset =
                        std::chrono::nanoseconds(0)) {
            mixxx::Time::addTestTime(getTimeThreshold() + offset);
        }
    };

  private:
    // If a new value is received within this amount of time, jump to it
    // regardless. This allows quickly whipping controls to work while retaining
    // the benefits of soft-takeover for slower movements.  Setting this too
    // high will defeat the purpose of soft-takeover.
    static constexpr ClockT::duration kSubsequentValueOverrideTime = std::chrono::milliseconds(50);
    static constexpr ClockT::time_point kFirstValueTime = ClockT::time_point::min();

    ClockT::time_point m_time{kFirstValueTime};
    double m_prevParameter{0};
    double m_dThreshold{kDefaultTakeoverThreshold};
};

class SoftTakeoverCtrl {
  public:
    SoftTakeoverCtrl() = default;

    // Enable soft-takeover for the given Control.
    // This does nothing on a control that already has soft-takeover enabled.
    void enable(gsl::not_null<ControlPotmeter*> pControl);
    // Disable soft-takeover for the given Control
    void disable(ControlObject* control) {
        m_softTakeoverHash.erase(control);
    }
    // Check to see if the new value for the Control should be ignored
    bool ignore(ControlObject* pControl, double newParameter) {
        auto it = m_softTakeoverHash.find(pControl);
        if (it == m_softTakeoverHash.end()) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(pControl) {
            return false;
        }
        auto& [coKey, refSoftTakeover] = *it;
        return refSoftTakeover.ignore(*pControl, newParameter);
    }
    bool willIgnore(ControlObject* pControl, double newParameter) {
        auto it = m_softTakeoverHash.find(pControl);
        if (it == m_softTakeoverHash.end()) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(pControl) {
            return false;
        }
        auto currentTime = SoftTakeover::ClockT::now();
        auto& [coKey, refSoftTakeover] = *it;
        return refSoftTakeover.willIgnore(*pControl, newParameter, currentTime);
    }
    // Ignore the next supplied parameter
    void ignoreNext(ControlObject* pControl) {
        auto it = m_softTakeoverHash.find(pControl);
        if (it == m_softTakeoverHash.end()) {
            return;
        }
        auto& [coKey, refSoftTakeover] = *it;
        refSoftTakeover.ignoreNext();
    }

  private:
    // ControlObjects are borrowed. They must outlive this object.
    // Note that even though we can only enable softTakeover on
    // `ControlPotmeter`s, we store the base ControlObject to not force the user
    // to downcast for `disable()` and `ignore()`/`ignoreNext()`.
    std::unordered_map<ControlObject*, SoftTakeover> m_softTakeoverHash;
};
