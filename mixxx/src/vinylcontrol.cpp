#include "vinylcontrol.h"
#include "controlobjectthread.h"

VinylControl::VinylControl(ConfigObject<ConfigValue> * pConfig, const char * _group)
{
    m_pConfig = pConfig;
    group = _group;
    iRIAACorrection = 0;

    // Get Control objects
    playPos             = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "playposition")));    //Range: 0.0 to 1.0
    controlScratch      = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "scratch2")));
    rateSlider          = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "rate")));    //Range -1.0 to 1.0
    playButton          = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "play")));
    reverseButton       = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "reverse")));
    duration            = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "duration")));
    mode                = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "VinylMode")));
	enabled     		= new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "vinylcontrol")));
    rateRange           = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "rateRange")));
    timecodeQuality     = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "VinylControlQuality")));
    timecodeInputL      = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "VinylControlInputL")));
    timecodeInputR      = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "VinylControlInputR")));
    //vinylStatus			= new ControlObject(ConfigKey(group,"VinylStatus"));
    vinylStatus     = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "VinylStatus")));
    rateDir         = new ControlObjectThread(ControlObject::getControl(ConfigKey(group, "rate_dir")));
    ControlObjectThread *samplerate = new ControlObjectThread(ControlObject::getControl(ConfigKey("[Master]", "samplerate")));
    iSampleRate = samplerate->get();

    dVinylPitch = 0.0f;
    dVinylPosition = 0.0f;
    dVinylScratch = 0.0f;
    dDriftControl   = 0.0f;
    fRateRange = 0.0f;

    //Get the vinyl type
    strVinylType = m_pConfig->getValueString(ConfigKey("[VinylControl]","strVinylType"));

    //Get the lead-in time
    iLeadInTime = m_pConfig->getValueString(ConfigKey("[VinylControl]","LeadInTime")).toInt();

    //RIAA correction
    iRIAACorrection =  m_pConfig->getValueString(ConfigKey("[VinylControl]","InputRIAACorrection")).toInt();

    //Vinyl control mode
    iVCMode = m_pConfig->getValueString(ConfigKey("[VinylControl]","Mode")).toInt();

    //Enabled or not
    bIsEnabled = m_pConfig->getValueString(ConfigKey(group,"vinylcontrol")).toInt();
    
    //Gain
    ControlObject::getControl(ConfigKey("[VinylControl]", "VinylControlGain"))->set(
    	m_pConfig->getValueString(ConfigKey("[VinylControl]","VinylControlGain")).toInt());
}

void VinylControl::ToggleVinylControl(bool enable)
{
    bIsEnabled = enable;
    if (m_pConfig)
        m_pConfig->set(ConfigKey(group,"vinylcontrol"), ConfigValue((int)enable));

    enabled->slotSet(enable);

    //Reset the scratch control to make sure we don't get stuck moving forwards or backwards.
    //actually that might be a good thing
    //if (!enable)
    //    controlScratch->slotSet(0.0f);
}

VinylControl::~VinylControl()
{

}

float VinylControl::getSpeed()
{
    return dVinylScratch;
}

//Returns some sort of indication of the vinyl's signal strength.
//Range of fTimecodeStrength should be 0.0 to 1.0
/*
float VinylControl::getTimecodeStrength()
{
    return fTimecodeStrength;
}
*/
