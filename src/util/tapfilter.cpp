#include "util/tapfilter.h"

TapFilter::TapFilter(QObject* pParent, int filterLength, int maxInterval)
        : QObject(pParent),
          m_mean(MovingInterquartileMean(filterLength)),
          m_iMaxInterval(maxInterval) {
    m_timer.start();
}

TapFilter::~TapFilter() {
}

void TapFilter::tap() {
    int millisElapsed = m_timer.restart();
    if (millisElapsed <= m_iMaxInterval) {
        emit(tapped(m_mean.insert(millisElapsed), m_mean.size()));
    } else {
        // Reset the filter
        m_mean.clear();
    }
}
