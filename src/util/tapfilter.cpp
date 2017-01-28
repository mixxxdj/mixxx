#include "util/tapfilter.h"

TapFilter::TapFilter(QObject* pParent, int filterLength, mixxx::Duration maxInterval)
        : QObject(pParent),
          m_mean(MovingInterquartileMean(filterLength)),
          m_maxInterval(maxInterval) {
    m_timer.start();
}

TapFilter::~TapFilter() {
}

void TapFilter::tap() {
    mixxx::Duration elapsed = m_timer.restart();
    if (elapsed <= m_maxInterval) {
        emit(tapped(m_mean.insert(elapsed.toDoubleMillis()), m_mean.size()));
    } else {
        // Reset the filter
        m_mean.clear();
    }
}
