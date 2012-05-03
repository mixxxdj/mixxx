#include "vinylcontrol.h"
#include "controlobjectthread.h"

VinylControl::VinylControl(ConfigObject<ConfigValue> * pConfig, QString group)
{
    m_pConfig = pConfig;
    m_group = group;
    iRIAACorrection = 0;

    iSampleRate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toULong();

    // Get Control objects
    playPos             = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "playposition")));    //Range: -.14 to 1.14
    trackSamples        = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "track_samples")));
    trackSampleRate     = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "track_samplerate")));
    vinylSeek           = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "vinylcontrol_seek")));
    controlScratch      = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "scratch2")));
    rateSlider          = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "rate")));    //Range -1.0 to 1.0
    playButton          = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "play")));
    reverseButton       = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "reverse")));
    duration            = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "duration")));
    mode                = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "vinylcontrol_mode")));
    enabled             = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "vinylcontrol_enabled")));
    wantenabled         = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "vinylcontrol_wantenabled")));
    cueing              = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "vinylcontrol_cueing")));
    scratching          = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "vinylcontrol_scratching")));
    rateRange           = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "rateRange")));
    vinylStatus     = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "vinylcontrol_status")));
    rateDir         = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "rate_dir")));
    loopEnabled     = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "loop_enabled")));
    signalenabled     = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "vinylcontrol_signal_enabled")));

    dVinylPitch = 0.0f;
    dVinylPosition = 0.0f;
    dVinylScratch = 0.0f;
    dDriftControl   = 0.0f;
    fRateRange = 0.0f;
    m_fTimecodeQuality = 0.0f;

    //Get the vinyl type
    strVinylType = m_pConfig->getValueString(ConfigKey(group,"vinylcontrol_vinyl_type"));

    //Get the vinyl speed
    strVinylSpeed = m_pConfig->getValueString(ConfigKey(group,"vinylcontrol_speed_type"));

    //Get the lead-in time
    iLeadInTime = m_pConfig->getValueString(ConfigKey("[VinylControl]","lead_in_time")).toInt();

    //RIAA correction
    iRIAACorrection =  m_pConfig->getValueString(ConfigKey("[VinylControl]","InputRIAACorrection")).toInt();

    //Vinyl control mode
    iVCMode = m_pConfig->getValueString(ConfigKey("[VinylControl]","mode")).toInt();

    //Enabled or not -- load from saved value in case vinyl control is restarting 
    bIsEnabled = wantenabled->get();

    //Gain
    ControlObject::getControl(ConfigKey("[VinylControl]", "gain"))->set(
        m_pConfig->getValueString(ConfigKey("[VinylControl]","gain")).toInt());
}

void VinylControl::ToggleVinylControl(bool enable)
{
    bIsEnabled = enable;
    if (m_pConfig)
    {
        m_pConfig->set(ConfigKey(m_group,"vinylcontrol_enabled"), ConfigValue((int)enable));
    }

    enabled->slotSet(enable);

    //Reset the scratch control to make sure we don't get stuck moving forwards or backwards.
    //actually that might be a good thing
    //if (!enable)
    //    controlScratch->slotSet(0.0f);
}

VinylControl::~VinylControl()
{
    bool wasEnabled = bIsEnabled;
    enabled->slotSet(false);
    vinylStatus->slotSet(VINYL_STATUS_DISABLED);
    if (wasEnabled)
    {
        //if vinyl control is just restarting, indicate that it should
        //be enabled
        wantenabled->slotSet(true);
    }
}

float VinylControl::getSpeed()
{
    return dVinylScratch;
}

/** Returns some sort of indication of the vinyl's signal quality.
    Range of m_fTimecodeQuality should be 0.0 to 1.0 */
float VinylControl::getTimecodeQuality()
{
    return m_fTimecodeQuality;
}

unsigned char* VinylControl::getScopeBytemap()
{
    return NULL;
}

float VinylControl::getAngle()
{
    return -1.0;
}
