#ifndef TAPFILTER_H
#define TAPFILTER_H

#include <QByteArrayData>
#include <QMutex>
#include <QObject>
#include <QString>

#include "util/duration.h"
#include "util/movinginterquartilemean.h"
#include "util/performancetimer.h"
#include "util/types.h"

class TapFilter : public QObject {
    Q_OBJECT
  public:
    TapFilter(QObject *pParent, int filterLength, mixxx::Duration maxInterval);
    virtual ~TapFilter();

  public slots:
    void tap();

  signals:
    void tapped(double averageLength, int numSamples);

  private:
    PerformanceTimer m_timer;
    MovingInterquartileMean m_mean;
    mixxx::Duration m_maxInterval;
    QMutex m_mutex;
};

#endif /* TAPFILTER_H */
