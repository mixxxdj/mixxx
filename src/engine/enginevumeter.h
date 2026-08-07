#pragma once

#include <cstddef>

#include "control/controlobject.h"
#include "control/pollingcontrolproxy.h"
#include "engine/engineobject.h"

class EngineVuMeter : public EngineObject {
    Q_OBJECT
  public:
    enum class MeterMode {
        DjPeak = 0,
        VuRms = 1,
    };

    EngineVuMeter(const QString& group,
            const QString& legacyGroup = QString(),
            bool createLegacyAliases = true);

    virtual void process(CSAMPLE* pInOut, const std::size_t bufferSize);

    void reset();

    static const ConfigKey& modeConfigKey();
    static MeterMode modeFromValue(double value);
    static double modeToValue(MeterMode mode);

  private:
    struct MeterInput {
        CSAMPLE peakLeft;
        CSAMPLE peakRight;
        CSAMPLE squareSumLeft;
        CSAMPLE squareSumRight;
        std::size_t frameCount;
        double secondsElapsed;
    };

    struct StereoLevel {
        CSAMPLE left;
        CSAMPLE right;
    };

    // Shared per-channel level state and smoothing structure for the level
    // calculators. Each concrete calculator only defines how a raw window
    // measurement maps to a target level and which ballistics smooth the
    // displayed value toward that target.
    class MeterCalculatorBase {
      public:
        virtual ~MeterCalculatorBase() = default;

        StereoLevel process(const MeterInput& input);
        void reset();

      protected:
        // Maps the raw per-channel window measurements to a normalized 0..1
        // level. Which inputs are relevant depends on the meter mode
        // (peak amplitude vs. RMS over the window).
        virtual CSAMPLE targetLevel(
                CSAMPLE peak, CSAMPLE squareSum, std::size_t frameCount) const = 0;
        // Moves currentVolume toward targetLevel using the mode-specific
        // attack/release ballistics.
        virtual void smooth(CSAMPLE& currentVolume,
                CSAMPLE targetLevel,
                double secondsElapsed) const = 0;

      private:
        CSAMPLE m_volumeLeft = 0;
        CSAMPLE m_volumeRight = 0;
    };

    class DjPeakMeterCalculator : public MeterCalculatorBase {
      protected:
        CSAMPLE targetLevel(CSAMPLE peak,
                CSAMPLE squareSum,
                std::size_t frameCount) const override;
        void smooth(CSAMPLE& currentVolume,
                CSAMPLE targetLevel,
                double secondsElapsed) const override;
    };

    class VuRmsMeterCalculator : public MeterCalculatorBase {
      protected:
        CSAMPLE targetLevel(CSAMPLE peak,
                CSAMPLE squareSum,
                std::size_t frameCount) const override;
        void smooth(CSAMPLE& currentVolume,
                CSAMPLE targetLevel,
                double secondsElapsed) const override;
    };

    class MeterCalculator {
      public:
        MeterMode mode() const {
            return m_mode;
        }
        void setMode(MeterMode mode);
        StereoLevel process(const MeterInput& input);
        void reset();

      private:
        MeterCalculatorBase& activeCalculator();

        MeterMode m_mode = MeterMode::DjPeak;
        DjPeakMeterCalculator m_djPeakMeter;
        VuRmsMeterCalculator m_vuRmsMeter;
    };

    void updateMeterMode();
    void setMeterControls(StereoLevel level);
    void setPeakIndicatorControls();

    ControlObject m_vuMeter;
    ControlObject m_vuMeterLeft;
    ControlObject m_vuMeterRight;
    CSAMPLE m_peakVolumeL;
    CSAMPLE m_peakVolumeR;
    CSAMPLE m_squareSumL;
    CSAMPLE m_squareSumR;
    std::size_t m_framesCalculated;
    MeterCalculator m_meterCalculator;

    ControlObject m_peakIndicator;
    ControlObject m_peakIndicatorLeft;
    ControlObject m_peakIndicatorRight;
    std::size_t m_peakDurationFramesL;
    std::size_t m_peakDurationFramesR;

    PollingControlProxy m_sampleRate;
    PollingControlProxy m_meterMode;
};
