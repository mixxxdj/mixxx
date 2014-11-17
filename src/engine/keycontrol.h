#ifndef KEYCONTROL_H
#define KEYCONTROL_H

#include "engine/enginecontrol.h"
#include "tapfilter.h"

class ControlObject;
class ControlPotmeter;
class ControlPushButton;

class KeyControl : public EngineControl {
    Q_OBJECT
  public:
    KeyControl(QString group, ConfigObject<ConfigValue>* pConfig);
    virtual ~KeyControl();

    // Returns a value describing the pitch adjustment measured in octaves. A
    // pitch adjustment of 0 means no change should take place.
    double getPitchAdjustOctaves();

    double getKey();

    void collectFeatures(GroupFeatureState* pGroupFeatures) const;

  private slots:
    void slotSetEngineKey(double);
    void slotFileKeyChanged(double);
    void slotPitchChanged(double);
    void slotRateChanged();
    void slotSyncKey(double);

  private:
    bool syncKey(EngineBuffer* pOtherEngineBuffer);

    // The previous rate slider rate.
    double m_dOldRate;
    bool m_bOldKeylock;
    double m_dPitchCompensation;
    double m_dPitchCompensationOldPitch;

    // ControlObjects that come from EngineBuffer
    ControlObject* m_pRateSlider;
    ControlObject* m_pRateRange;
    ControlObject* m_pRateDir;
    ControlObject* m_pKeylock;
    ControlPotmeter* m_pPitch;
    ControlPushButton* m_pButtonSyncKey;

    /** The current loaded file's detected key */
    ControlObject* m_pFileKey;

    /** The current effective key of the engine */
    ControlObject* m_pEngineKey;
    ControlPotmeter* m_pEngineKeyDistance;

    TrackPointer m_pTrack;
};

#endif // KEYCONTROL_H
