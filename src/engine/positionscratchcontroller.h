#pragma once

#include <QObject>
#include <QString>

#include "audio/frame.h"

class ControlObject;
class RateIIFilter;
class VelocityController;

class PositionScratchController : public QObject {
    Q_OBJECT
  public:
    PositionScratchController(const QString& group);
    virtual ~PositionScratchController();

    void process(double currentSample,
            double releaseRate,
            std::size_t bufferSize,
            double baseSampleRate,
            int wrappedAround,
            mixxx::audio::FramePos trigger,
            mixxx::audio::FramePos target);
    bool isEnabled();
    double getRate();
    void notifySeek(mixxx::audio::FramePos position);

  private:
    const QString m_group;
    ControlObject* m_pScratchEnable;
    ControlObject* m_pScratchPos;
    ControlObject* m_pMainSampleRate;
    VelocityController* m_pVelocityController;
    RateIIFilter* m_pRateIIFilter;
    bool m_isScratching;
    bool m_inertiaEnabled;
    double m_prevSamplePos;
    double m_samplePosDeltaSum;
    double m_scratchTargetDelta;
    double m_scratchStartPos;
    double m_rate;
    double m_moveDelay;
    double m_mouseSampleTime;
};
