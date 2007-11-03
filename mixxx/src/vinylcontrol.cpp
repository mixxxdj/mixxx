#include "vinylcontrol.h"
#include "controlobjectthreadmain.h"

VinylControl::VinylControl(ConfigObject<ConfigValue> * pConfig, const char * _group)
{
    m_pConfig = pConfig;
    group = _group;

    iSampleRate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt();
    iRIAACorrection = 0;

    // Get Control objects
    playPos                 = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "playposition")));    //Range: 0.0 to 1.0
    controlScratch  = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "scratch")));
    rateSlider              = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rate")));    //Range -1.0 to 1.0
    playButton          = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "play")));
    reverseButton       = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "reverse")));
    duration                = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "duration")));
    mode                = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[VinylControl]", "Mode")));
    enabled             = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[VinylControl]", "Enabled")));

    dVinylPitch = 0.0f;
    dVinylPosition = 0.0f;
    dVinylScratch = 0.0f;
    dDriftControl   = 0.0f;
    fTimecodeStrength = 0.0f;

    //Get the vinyl type
    strVinylType = m_pConfig->getValueString(ConfigKey("[VinylControl]","strVinylType"));

    //Get the lead-in time
    iLeadInTime = m_pConfig->getValueString(ConfigKey("[VinylControl]","LeadInTime")).toInt();

    //RIAA correction
    iRIAACorrection =  m_pConfig->getValueString(ConfigKey("[VinylControl]","InputRIAACorrection")).toInt();

    //Vinyl control mode
    iVCMode = m_pConfig->getValueString(ConfigKey("[VinylControl]","Mode")).toInt();
    
    //Enabled or not
    bIsEnabled = m_pConfig->getValueString(ConfigKey("[VinylControl]","Enabled")).toInt();

}

void VinylControl::ToggleVinylControl(bool enable)
{
    bIsEnabled = enable;
    if (m_pConfig)
        m_pConfig->set(ConfigKey("[VinylControl]","Enabled"), ConfigValue((int)enable));
    
    enabled->slotSet(enable);
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
float VinylControl::getTimecodeStrength()
{
    return fTimecodeStrength;
}
