#ifndef KEYCONTROL_H
#define KEYCONTROL_H

#include "engine/enginecontrol.h"
#include "tapfilter.h"
#include "control/controlvalue.h"

class ControlObject;
class ControlPotmeter;
class ControlPushButton;

class KeyControl : public EngineControl {
    Q_OBJECT
  public:

    struct PitchTempoRatio {
        // this is the calculated value used by engine buffer for pitch
        // by default is is equal to the tempoRatio set by the speed slider
        double pitchRatio;
        // this is the value of the speed slider and speed slider
        // effecting controls at the moment of calculation
        double tempoRatio;
        bool keylock;
    };

    KeyControl(QString group, ConfigObject<ConfigValue>* pConfig);
    virtual ~KeyControl();

    // Returns a struct, with the results of the last pitch and tempo calculations
    KeyControl::PitchTempoRatio getPitchTempoRatio() const;

    double getKey();

    void collectFeatures(GroupFeatureState* pGroupFeatures) const;

  private slots:
    void slotSetEngineKey(double);
    void slotSetEngineKeyDistance(double);
    void slotFileKeyChanged(double);
    void slotPitchChanged(double);
    void slotRateChanged();
    void slotSyncKey(double);
    void slotResetKey(double);
    void slotPitchAndKeylockModeChanged(double);

  private:
    void setEngineKey(double key, double key_distance);
    bool syncKey(EngineBuffer* pOtherEngineBuffer);
    void updateKeyCOs(double fileKeyNumeric, double pitch);

    // The previous rate slider rate.
    bool m_bOldKeylock;
    // Pitch Ratio caused bay the speed slider (m_pRateSlider)
    double m_speedSliderPitchRatio;

    // ControlObjects that come from EngineBuffer
    ControlObject* m_pRateSlider;
    ControlObject* m_pRateRange;
    ControlObject* m_pRateDir;
    ControlObject* m_pKeylock;
    ControlPotmeter* m_pPitch;
    ControlPushButton* m_pButtonSyncKey;
    ControlPushButton* m_pButtonResetKey;
    ControlPushButton* m_pitchAndKeylockMode;

    /** The current loaded file's detected key */
    ControlObject* m_pFileKey;

    /** The current effective key of the engine */
    ControlObject* m_pEngineKey;
    ControlPotmeter* m_pEngineKeyDistance;

    TrackPointer m_pTrack;

    int m_iPitchAndKeylockMode;
    ControlValueAtomic<struct PitchTempoRatio> m_pitchRateInfo;
};

#endif // KEYCONTROL_H
