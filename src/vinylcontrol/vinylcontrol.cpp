#include "vinylcontrol/vinylcontrol.h"
#include "control/controlproxy.h"
#include "control/controlobject.h"

VinylControl::VinylControl(UserSettingsPointer pConfig, const QString& group)
        : m_pConfig(pConfig),
          m_group(group),
          m_iLeadInTime(m_pConfig->getValueString(
                                         ConfigKey(group, "vinylcontrol_lead_in_time"))
                                .toInt()),
          m_dVinylPosition(0.0),
          m_fTimecodeQuality(0.0f) {
    // Get Control objects
    m_pVinylControlInputGain = new ControlProxy(VINYL_PREF_KEY, "gain", this);

    bool gainOk = false;
    double gain = m_pConfig->getValueString(ConfigKey(VINYL_PREF_KEY, "gain"))
            .toDouble(&gainOk);
    m_pVinylControlInputGain->set(gainOk ? gain : 1.0);

    // Range: 0 to 1.0
    playPos = new ControlProxy(group, "playposition", this);
    trackSamples = new ControlProxy(group, "track_samples", this);
    trackSampleRate = new ControlProxy(group, "track_samplerate", this);
    vinylSeek = new ControlProxy(group, "vinylcontrol_seek", this);
    m_pVCRate = new ControlProxy(group, "vinylcontrol_rate", this);
    m_pRateRatio = new ControlProxy(group, "rate_ratio", this);
    playButton = new ControlProxy(group, "play", this);
    duration = new ControlProxy(group, "duration", this);
    mode = new ControlProxy(group, "vinylcontrol_mode", this);
    enabled = new ControlProxy(group, "vinylcontrol_enabled", this);
    wantenabled = new ControlProxy(
            group, "vinylcontrol_wantenabled", this);
    cueing = new ControlProxy(group, "vinylcontrol_cueing", this);
    scratching = new ControlProxy(group, "vinylcontrol_scratching", this);
    vinylStatus = new ControlProxy(group, "vinylcontrol_status", this);
    loopEnabled = new ControlProxy(group, "loop_enabled", this);
    signalenabled = new ControlProxy(
            group, "vinylcontrol_signal_enabled", this);
    reverseButton = new ControlProxy(group, "reverse", this);

    //Enabled or not -- load from saved value in case vinyl control is restarting
    m_bIsEnabled = wantenabled->get() > 0.0;
}

bool VinylControl::isEnabled() {
    return m_bIsEnabled;
}

void VinylControl::toggleVinylControl(bool enable) {
    if (m_pConfig) {
        m_pConfig->set(ConfigKey(m_group,"vinylcontrol_enabled"), ConfigValue((int)enable));
    }

    enabled->slotSet(enable);

    // Reset the scratch control to make sure we don't get stuck moving forwards or backwards.
    // actually that might be a good thing
    //if (!enable)
    //    controlScratch->slotSet(0.0);
}

VinylControl::~VinylControl() {
    bool wasEnabled = m_bIsEnabled;
    enabled->slotSet(false);
    vinylStatus->slotSet(VINYL_STATUS_DISABLED);
    if (wasEnabled) {
        // if vinyl control is just restarting, indicate that it should
        // be enabled
        wantenabled->slotSet(true);
    }
}
