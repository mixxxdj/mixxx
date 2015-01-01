#ifndef TAPFILTER_H
#define TAPFILTER_H

#include <QObject>
#include <QTime>

#include "movingtruncatediqm.h"
#include "util/types.h"

class TapFilter : public QObject {
    Q_OBJECT
  public:
    TapFilter(QObject *pParent, int filterLength, int maxInterval);
    virtual ~TapFilter();
    void tap();

  signals:
    void tapped(double averageLength, int numSamples);

  private:
    QTime m_timer;
    MovingTruncatedIQM m_mean;
    const int m_iMeanWindowSize;
    int m_iMaxInterval;
};

#endif /* TAPFILTER_H */
