#pragma once

#include <QMap>
#include <cmath>

#include "effects/backends/effectprocessor.h"
#include "engine/filters/enginefilterpansingle.h"
#include "util/class.h"
#include "util/types.h"

// This class provides a float value that cannot be increased or decreased
// by more than a given value to avoid clicks.
// Finally I use it with only one value but i wonder if it can be useful
// somewhere else (I hear clicks when I change the period of flanger for example).
class RampedSample {
  public:
    constexpr RampedSample()
            : ramped(false),
              maxDifference(1.0f),
              currentValue(0),
              initialized(false) {
    }

    constexpr void setRampingThreshold(float newMaxDifference) {
        maxDifference = newMaxDifference;
    }

    constexpr void setWithRampingApplied(float newValue) {
        if (!initialized) {
            currentValue = newValue;
            initialized = true;
        } else {
            float difference = newValue - currentValue;
            if (abs(difference) > maxDifference) {
                currentValue += difference / abs(difference) * maxDifference;
                ramped = true;
            } else {
                currentValue = newValue;
                ramped = false;
            }
        }
    }

    constexpr operator float() {
        return currentValue;
    }

    bool ramped;

  private:
    float maxDifference;
    float currentValue;
    bool initialized;
};

static constexpr int panMaxDelay = 3300; // allows a 30 Hz filter at 97346;
// static const int panMaxDelay = 50000; // high for debug;

class AutoPanGroupState : public EffectState {
  public:
    AutoPanGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
              time(0),
              pDelay(std::make_unique<EngineFilterPanSingle<panMaxDelay>>()),
              m_dPreviousPeriod(-1.0) {
    }
    ~AutoPanGroupState() override = default;

    unsigned int time;
    RampedSample frac;
    std::unique_ptr<EngineFilterPanSingle<panMaxDelay>> pDelay;
    double m_dPreviousPeriod;
};

class AutoPanEffect : public EffectProcessorImpl<AutoPanGroupState> {
  public:
    AutoPanEffect() = default;
    ~AutoPanEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            AutoPanGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

    double computeLawCoefficient(double position);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pSmoothingParameter;
    EngineEffectParameterPointer m_pPeriodParameter;
    EngineEffectParameterPointer m_pWidthParameter;

    DISALLOW_COPY_AND_ASSIGN(AutoPanEffect);
};
