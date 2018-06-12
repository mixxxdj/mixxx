#ifndef MIXXX_UTIL_RAMPINGVALUE_H
#define MIXXX_UTIL_RAMPINGVALUE_H

// RampingValue is a utility class for numeric values that need to be smoothly
// ramped across each frame or sample of an audio buffer. It compares the
// current value of a parameter at the time of an audio engine callback to that
// parameter's value at the last cycle of the audio engine and smoothly
// interpolates the intermediate values across the span of the buffer.

// To use a RampingValue, store it as a variable that persists across audio
// engine callbacks. The constructor takes the number of steps that will be
// used when iterating over the audio buffer, which is typically the number of
// frames or samples in the audio engine's buffer. At the start of each audio
// engine cycle, before the parameter is used, call
// RampingValue::setCurrentCallbackValue with the parameter value received from
// outside the audio engine thread. Then, when looping over the audio buffer,
// get the interpolated value with RampingValue::rampedValue
template <typename T>
class RampingValue {
  public:
    RampingValue(int stepsPerCallback)
        : m_iStepsPerCallback(stepsPerCallback),
          m_iStepsSinceCallbackStart(0),
          m_currentCallbackValue(0),
          m_lastCallbackValue(0) {
    }

    void setCurrentCallbackValue(const T& currentValue) {
        m_lastCallbackValue = m_currentCallbackValue;
        m_currentCallbackValue = currentValue;
        DEBUG_ASSERT(m_iStepsSinceCallbackStart == m_iStepsPerCallback
                     || m_iStepsSinceCallbackStart == 0);
        m_iStepsSinceCallbackStart = 0;
    }

    T rampedValue() {
        m_iStepsSinceCallbackStart++;
        DEBUG_ASSERT(m_iStepsSinceCallbackStart <= m_iStepsPerCallback);
        T stepSize = (m_currentCallbackValue - m_lastCallbackValue) / m_iStepsPerCallback;
        return m_lastCallbackValue + stepSize * m_iStepsSinceCallbackStart;
    }

    // FIXME: This should not be necessary. m_iStepsPerCallback should only need
    // to be set on initialization. This is a temporary hack around EffectStates
    // not getting initialized with the actual parameters that the audio engine
    // is using.
    void setStepsPerCallback(int stepsPerCallback) {
        m_iStepsPerCallback = stepsPerCallback;
    }

  private:
    int m_iStepsPerCallback;
    int m_iStepsSinceCallbackStart;
    T m_currentCallbackValue;
    T m_lastCallbackValue;
};

#endif // MIXXX_UTIL_RAMPINGVALUE_H
