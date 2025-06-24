#pragma once

#include "engine/controls/enginecontrol.h"
#include "util/parented_ptr.h"

class ControlObject;
class ControlProxy;
class ControlPotmeter;
class ControlPushButton;

class KeyControl : public EngineControl {
    Q_OBJECT
  public:

    struct PitchTempoRatio {
        // This is the calculated value used by engine buffer for pitch.
        // By default it is equal to the tempoRatio set by the speed slider
        double pitchRatio;
        // This is the value of the speed slider and speed slider
        // affecting controls at the moment of calculation
        double tempoRatio;
        // the offset factor to the natural pitch set by the rate slider
        double pitchTweakRatio;
        bool keylock;
    };

    KeyControl(const QString& group, UserSettingsPointer pConfig);

    // Returns a struct, with the results of the last pitch and tempo calculations
    KeyControl::PitchTempoRatio getPitchTempoRatio();

    double getKey();

  private slots:
    void slotSetEngineKey(double);
    void slotSetEngineKeyDistance(double);
    void slotFileKeyChanged(double);
    void slotPitchChanged(double);
    void slotPitchAdjustChanged(double);
    void slotRateChanged();
    void slotSyncKey(double);
    void slotResetKey(double);

  private:
    void setEngineKey(double key, double key_distance);
    bool syncKey(EngineBuffer* pOtherEngineBuffer);
    void updateKeyCOs(double fileKeyNumeric, double pitchOctaves);
    void updatePitch();
    void updatePitchAdjust();
    void updateRate();

    // ControlObjects that come from EngineBuffer
    parented_ptr<ControlProxy> m_pRateRatio;

    parented_ptr<ControlProxy> m_pVCRate;
    parented_ptr<ControlProxy> m_pVCEnabled;

    parented_ptr<ControlProxy> m_pKeylock;
    std::unique_ptr<ControlPotmeter> m_pPitch;
    std::unique_ptr<ControlPotmeter> m_pPitchAdjust;
    std::unique_ptr<ControlPushButton> m_pButtonSyncKey;
    std::unique_ptr<ControlPushButton> m_pButtonResetKey;
    std::unique_ptr<ControlPushButton> m_keylockMode;
    std::unique_ptr<ControlPushButton> m_keyunlockMode;

    // The current loaded file's detected key
    std::unique_ptr<ControlObject> m_pFileKey;

    // The current effective key of the engine
    std::unique_ptr<ControlObject> m_pEngineKey;
    std::unique_ptr<ControlPotmeter> m_pEngineKeyDistance;

    struct PitchTempoRatio m_pitchRateInfo;
    QAtomicInt m_updatePitchRequest;
    QAtomicInt m_updatePitchAdjustRequest;
    QAtomicInt m_updateRateRequest;
};
