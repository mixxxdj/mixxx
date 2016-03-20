#include "vinylcontrol/vinylcontrol.h"
#include "controlobjectslave.h"
#include "controlobject.h"

VinylControl::VinylControl(UserSettingsPointer pConfig, QString group)
        : m_pConfig(pConfig),
          m_group(group),
          m_iLeadInTime(m_pConfig->getValueString(
                  ConfigKey(group, "vinylcontrol_lead_in_time")).toInt()),
          m_dVinylPosition(0.0),
          m_fTimecodeQuality(0.0f) {
    // Get Control objects
    m_pVinylControlInputGain = new ControlObjectSlave(VINYL_PREF_KEY, "gain", this);

    bool gainOk = false;
    double gain = m_pConfig->getValueString(ConfigKey(VINYL_PREF_KEY, "gain"))
            .toDouble(&gainOk);
    m_pVinylControlInputGain->set(gainOk ? gain : 1.0);

    // Range: 0 to 1.0
    playPos = new ControlObjectSlave(group, "playposition", this);
    trackSamples = new ControlObjectSlave(group, "track_samples", this);
    trackSampleRate = new ControlObjectSlave(group, "track_samplerate", this);
    vinylSeek = new ControlObjectSlave(group, "vinylcontrol_seek", this);
    m_pVCRate = new ControlObjectSlave(group, "vinylcontrol_rate", this);
    m_pRateSlider = new ControlObjectSlave(group, "rate", this);
    playButton = new ControlObjectSlave(group, "play", this);
    duration = new ControlObjectSlave(group, "duration", this);
    mode = new ControlObjectSlave(group, "vinylcontrol_mode", this);
    enabled = new ControlObjectSlave(group, "vinylcontrol_enabled", this);
    wantenabled = new ControlObjectSlave(
            group, "vinylcontrol_wantenabled", this);
    cueing = new ControlObjectSlave(group, "vinylcontrol_cueing", this);
    scratching = new ControlObjectSlave(group, "vinylcontrol_scratching", this);
    m_pRateRange = new ControlObjectSlave(group, "rateRange", this);
    vinylStatus = new ControlObjectSlave(group, "vinylcontrol_status", this);
    m_pRateDir = new ControlObjectSlave(group, "rate_dir", this);
    loopEnabled = new ControlObjectSlave(group, "loop_enabled", this);
    signalenabled = new ControlObjectSlave(
            group, "vinylcontrol_signal_enabled", this);
    reverseButton = new ControlObjectSlave(group, "reverse", this);

    //Enabled or not -- load from saved value in case vinyl control is restarting
    m_bIsEnabled = wantenabled->get() > 0.0;

    // Load VC pre-amp gain from the config.
    // TODO(rryan): Should probably live in VinylControlManager since it's not
    // specific to a VC deck.
    ControlObject::set(ConfigKey(VINYL_PREF_KEY, "gain"),
        m_pConfig->getValueString(ConfigKey(VINYL_PREF_KEY,"gain")).toInt());
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
