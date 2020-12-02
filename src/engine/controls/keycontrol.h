#pragma once

#include "engine/controls/enginecontrol.h"
#include "control/controlvalue.h"
#include "util/parented_ptr.h"

class ControlObject;
class ControlProxy;
class ControlPotmeter;
class ControlPushButton;

class KeyControl : public EngineControl {
    Q_OBJECT
  public:

    struct PitchTempoRatio {
        // this is the calculated value used by engine buffer for pitch
        // by default it is equal to the tempoRatio set by the speed slider
        double pitchRatio;
        // this is the value of the speed slider and speed slider
        // effecting controls at the moment of calculation
        double tempoRatio;
        // the offeset factor to the natural pitch set by the rate slider
        double pitchTweakRatio;
        bool keylock;
    };

    KeyControl(const QString& group, UserSettingsPointer pConfig);
    ~KeyControl() override;

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
    void updateKeyCOs(double fileKeyNumeric, double pitch);
    void updatePitch();
    void updatePitchAdjust();
    void updateRate();

    // ControlObjects that come from EngineBuffer
    parented_ptr<ControlProxy> m_pRateRatio;

    ControlProxy* m_pVCRate;
    ControlProxy* m_pVCEnabled;

    ControlProxy* m_pKeylock;
    ControlPotmeter* m_pPitch;
    ControlPotmeter* m_pPitchAdjust;
    ControlPushButton* m_pButtonSyncKey;
    ControlPushButton* m_pButtonResetKey;
    ControlPushButton* m_keylockMode;
    ControlPushButton* m_keyunlockMode;

    // The current loaded file's detected key
    ControlObject* m_pFileKey;

    // The current effective key of the engine
    ControlObject* m_pEngineKey;
    ControlPotmeter* m_pEngineKeyDistance;

    struct PitchTempoRatio m_pitchRateInfo;
    QAtomicInt m_updatePitchRequest;
    QAtomicInt m_updatePitchAdjustRequest;
    QAtomicInt m_updateRateRequest;
};
