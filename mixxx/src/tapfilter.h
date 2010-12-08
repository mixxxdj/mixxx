#ifndef TAPFILTER_H
#define TAPFILTER_H

#include <QObject>
#include <QTime>

#include "defs.h"

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
    CSAMPLE* m_pFilterBuffer;
    int m_iFilterLength;
    int m_iValidPresses;
    int m_iMaxInterval;
};

#endif /* TAPFILTER_H */
