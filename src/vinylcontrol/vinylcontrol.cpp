#include "vinylcontrol/vinylcontrol.h"
#include "controlobjectthread.h"
#include "controlobjectslave.h"
#include "controlobject.h"

VinylControl::VinylControl(ConfigObject<ConfigValue> * pConfig, QString group)
        : m_pConfig(pConfig),
          m_group(group),
          m_iLeadInTime(m_pConfig->getValueString(
              ConfigKey(group, "vinylcontrol_lead_in_time")).toInt()),
          m_dVinylPosition(0.0),
          m_fTimecodeQuality(0.0f) {
    // Get Control objects
    m_pVinylControlInputGain = new ControlObjectThread(VINYL_PREF_KEY, "gain");

    bool gainOk = false;
    double gain = m_pConfig->getValueString(ConfigKey(VINYL_PREF_KEY, "gain"))
            .toDouble(&gainOk);
    m_pVinylControlInputGain->set(gainOk ? gain : 1.0);

    playPos             = new ControlObjectThread(group, "playposition");    // Range: 0 to 1.0
    trackSamples        = new ControlObjectThread(group, "track_samples");
    trackSampleRate     = new ControlObjectThread(group, "track_samplerate");
    vinylSeek           = new ControlObjectThread(group, "vinylcontrol_seek");
    m_pVCRate = new ControlObjectSlave(group, "vinylcontrol_rate");
    m_pRateSlider = new ControlObjectSlave(group, "rate");
    playButton          = new ControlObjectThread(group, "play");
    duration            = new ControlObjectThread(group, "duration");
    mode                = new ControlObjectThread(group, "vinylcontrol_mode");
    enabled             = new ControlObjectThread(group, "vinylcontrol_enabled");
    wantenabled         = new ControlObjectThread(group, "vinylcontrol_wantenabled");
    cueing              = new ControlObjectThread(group, "vinylcontrol_cueing");
    scratching          = new ControlObjectThread(group, "vinylcontrol_scratching");
    rateRange           = new ControlObjectThread(group, "rateRange");
    vinylStatus         = new ControlObjectThread(group, "vinylcontrol_status");
    rateDir             = new ControlObjectThread(group, "rate_dir");
    loopEnabled         = new ControlObjectThread(group, "loop_enabled");
    signalenabled       = new ControlObjectThread(group, "vinylcontrol_signal_enabled");
    reverseButton       = new ControlObjectThread(group, "reverse");

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
        //if vinyl control is just restarting, indicate that it should
        //be enabled
        wantenabled->slotSet(true);
    }

    delete reverseButton;
    delete m_pVinylControlInputGain;
    delete playPos;
    delete trackSamples;
    delete trackSampleRate;
    delete vinylSeek;
    delete m_pVCRate;
    delete m_pRateSlider;
    delete playButton;
    delete duration;
    delete mode;
    delete enabled;
    delete wantenabled;
    delete cueing;
    delete scratching;
    delete rateRange;
    delete vinylStatus;
    delete rateDir;
    delete loopEnabled;
    delete signalenabled;
}
