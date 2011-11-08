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

    void process(double currentSample, bool paused, int iBufferSize);
    bool isEnabled();
    double getRate();
    void notifySeek(double currentSample);

  private:
    const QString m_group;
    ControlObject* m_pScratchEnable;
    ControlObject* m_pScratchPosition;
    VelocityController* m_pVelocityController;
    bool m_bScratching;
    bool m_bEnableInertia;
    double m_dLastPlaypos;
    double m_dPositionDeltaSum;
    int m_iScratchTime;
    double m_dRate;
};

#endif /* POSITIONSCRATCHCONTROLLER_H */
