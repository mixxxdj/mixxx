#ifndef KEYCONTROL_H
#define KEYCONTROL_H

#include "engine/enginecontrol.h"
#include "tapfilter.h"

class ControlObject;
class ControlPotmeter;
//class ControlPushButton;

class KeyControl : public EngineControl {
    Q_OBJECT

  public:
    KeyControl(const char* _group, ConfigObject<ConfigValue>* _config);
    virtual ~KeyControl();
    // Returns a value describing the pitch adjustment measured in octaves. A
    // pitch adjustment of 0 means no change should take place.
    double getPitchAdjust();
    double getKey();

  private slots:
    void slotSetEngineKey(double);
    void slotFileKeyChanged(double);
    void slotPitchChanged(double);

  private:
    // ControlObjects that come from EngineBuffer
    ControlObject* m_pPlayButton;
    ControlPotmeter* m_pPitch;

    /** The current loaded file's detected key */
    ControlObject* m_pFileKey;

    /** The current effective key of the engine */
    ControlObject* m_pEngineKey;

    TrackPointer m_pTrack;
};

#endif // KEYCONTROL_H
