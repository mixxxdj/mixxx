#ifndef POSITIONSCRATCHCONTROLLER_H
#define POSITIONSCRATCHCONTROLLER_H

#include <QObject>
#include <QString>

#include "controlobject.h"

class VelocityController;

class PositionScratchController : public QObject {
  public:
    PositionScratchController(const char* pGroup);
    virtual ~PositionScratchController();

    void process(double currentSample, int iBufferSize);
    bool isEnabled();
    double getRate();

  private:
    const QString m_group;
    ControlObject* m_pScratchEnable;
    ControlObject* m_pScratchPosition;
    VelocityController* m_pVelocityController;
    bool m_bScratching;
    int m_iScratchTime;
    double m_dRate;
};

#endif /* POSITIONSCRATCHCONTROLLER_H */
