#include "tapfilter.h"

TapFilter::TapFilter(QObject* pParent, int filterLength, int maxInterval)
        : QObject(pParent),
          m_pFilterBuffer(new CSAMPLE[filterLength]),
          m_iFilterLength(filterLength),
          m_iValidPresses(0),
          m_iMaxInterval(maxInterval) {
    m_timer.start();
}

TapFilter::~TapFilter() {
    delete [] m_pFilterBuffer;
}

void TapFilter::tap() {

    int millisElapsed = m_timer.restart();

    if (millisElapsed <= m_iMaxInterval) {
        // Move back in filter one sample
        for (int i = m_iFilterLength-1; i > 0; i--) {
            m_pFilterBuffer[i] = m_pFilterBuffer[i-1];
        }

        // Insert the sample in the filter
        m_pFilterBuffer[0] = millisElapsed;

        m_iValidPresses++;
        if (m_iValidPresses > m_iFilterLength)
            m_iValidPresses = m_iFilterLength;

        double temp = 0.;
        for (int i = 0; i < m_iValidPresses; ++i)
            temp += m_pFilterBuffer[i];
        temp /= m_iValidPresses;

        emit(tapped(temp, m_iValidPresses));
    } else {
        // Reset the filter
        m_iValidPresses = 0;
    }
}
