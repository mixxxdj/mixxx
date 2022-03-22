#pragma once

#include <QObject>
#include <QString>

#include "audio/frame.h"
#include "control/controlobject.h"

class VelocityController;
class RateIIFilter;

class PositionScratchController : public QObject {
    Q_OBJECT
  public:
    PositionScratchController(const QString& group);
    virtual ~PositionScratchController();

    void process(double currentSample, double releaseRate,
                 int iBufferSize, double baserate);
    bool isEnabled();
    double getRate();
    void notifySeek(mixxx::audio::FramePos position);

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
