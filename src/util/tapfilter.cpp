#include "util/tapfilter.h"

#include "moc_tapfilter.cpp"

TapFilter::TapFilter(QObject* pParent, int filterLength, mixxx::Duration maxInterval)
        : QObject(pParent),
          m_mean(MovingInterquartileMean(filterLength)),
          m_maxInterval(maxInterval) {
    m_timer.start();
}

TapFilter::~TapFilter() {
}

void TapFilter::tap() {
    QMutexLocker locker(&m_mutex);
    mixxx::Duration elapsed = m_timer.restart();
    if (elapsed <= m_maxInterval) {
        double averageLength = m_mean.insert(elapsed.toDoubleMillis());
        int numSamples = m_mean.size();
        locker.unlock();
        emit tapped(averageLength, numSamples);
    } else {
        // Reset the filter
        m_mean.clear();
    }
}
