#pragma once

#include <QObject>
#include <QString>

#include "audio/frame.h"

class ControlObject;
class ControlProxy;
class VelocityController;
class RateIIFilter;

class PositionScratchController : public QObject {
    Q_OBJECT
  public:
    PositionScratchController(const QString& group);
    // required for the forward-declarations of uniquq_pointers of
    // VelocityController and RateIIFilter
    ~PositionScratchController() override;

    void process(double currentSample,
            double releaseRate,
            std::size_t bufferSize,
            double baseSampleRate,
            int wrappedAround,
            mixxx::audio::FramePos trigger,
            mixxx::audio::FramePos target);
    bool isEnabled() const {
        // TODO return true only if m_rate is valid.
        return m_isScratching;
    }
    double getRate() const {
        return m_rate;
    }
    void notifySeek(mixxx::audio::FramePos position);
    void reset();

  private slots:
    void slotUpdateFilterParameters(double sampleRate);

  private:
    const QString m_group;
    std::unique_ptr<ControlObject> m_pScratchEnable;
    std::unique_ptr<ControlObject> m_pScratchPos;
    std::unique_ptr<ControlProxy> m_pMainSampleRate;
    std::unique_ptr<VelocityController> m_pVelocityController;
    std::unique_ptr<RateIIFilter> m_pRateIIFilter;
    bool m_isScratching;
    bool m_inertiaEnabled;
    double m_prevSamplePos;
    double m_seekSamplePos;
    double m_samplePosDeltaSum;
    double m_scratchTargetDelta;
    double m_scratchStartPos;
    double m_rate;
    double m_moveDelay;
    double m_scratchPosSampleTime;

    std::size_t m_bufferSize;

    double m_dt;
    double m_callsPerDt;
    double m_callsToStop;

    double m_p;
    double m_d;
    double m_f;
};
