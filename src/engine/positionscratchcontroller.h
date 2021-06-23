#pragma once

#include <QObject>
#include <QString>

#include "control/controlobject.h"

class VelocityController;
class RateIIFilter;

class PositionScratchController : public QObject {
  public:
    PositionScratchController(const QString& group);
    virtual ~PositionScratchController();

    void process(double currentSample, double releaseRate,
                 int iBufferSize, double baserate);
    bool isEnabled();
    double getRate();
    void notifySeek(double currentSample);

  private:
    const QString m_group;
    ControlObject* m_pScratchEnable;
    ControlObject* m_pScratchPosition;
    ControlObject* m_pMasterSampleRate;
    VelocityController* m_pVelocityController;
    RateIIFilter* m_pRateIIFilter;
    bool m_bScratching;
    bool m_bEnableInertia;
    double m_dLastPlaypos;
    double m_dPositionDeltaSum;
    double m_dTargetDelta;
    double m_dStartScratchPosition;
    double m_dRate;
    double m_dMoveDelay;
    double m_dMouseSampeTime;
};
