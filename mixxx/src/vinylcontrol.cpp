#include "vinylcontrol.h"


VinylControl::VinylControl(ConfigObject<ConfigValue> *pConfig, const char *_group)
{
	m_pConfig = pConfig;
	group = _group;
	bIsRunning		= true;	//Enabled by default - Albert
	
    iSampleRate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt();
    iRIAACorrection = 0;
		
	// Get Control objects
	playPos			= ControlObject::getControl(ConfigKey(group, "playposition")); //Range: 0.0 to 1.0
	controlScratch	= ControlObject::getControl(ConfigKey(group, "scratch"));
	rateSlider		= ControlObject::getControl(ConfigKey(group, "rate")); //Range -1.0 to 1.0
    playButton		= ControlObject::getControl(ConfigKey(group, "play"));
    reverseButton 	= ControlObject::getControl(ConfigKey(group, "reverse"));
	duration		= ControlObject::getControl(ConfigKey(group, "duration"));    
  
    dVinylPitch = 0.0f;
    dVinylPosition = 0.0f;
    dVinylScratch = 0.0f;
    dDriftControl   = 0.0f;

    //Get the vinyl type
    strVinylType = m_pConfig->getValueString(ConfigKey("[VinylControl]","strVinylType"));

	//Get relative mode
	bRelativeMode = (bool)m_pConfig->getValueString(ConfigKey("[VinylControl]","RelativeMode")).toInt();
	
	//Get scratch mode
	bScratchMode = (bool)m_pConfig->getValueString(ConfigKey("[VinylControl]","ScratchMode")).toInt();
	
	//Get the lead-in time
	iLeadInTime = m_pConfig->getValueString(ConfigKey("[VinylControl]","LeadInTime")).toInt();

	//RIAA correction
	iRIAACorrection =  m_pConfig->getValueString(ConfigKey("[VinylControl]","InputRIAACorrection")).toInt();

}

void VinylControl::ToggleVinylControl(bool enable)
{
	bIsRunning = enable;
	m_pConfig->set(ConfigKey("[VinylControl]","Enabled"), ConfigValue((int)enable));
}

VinylControl::~VinylControl()
{

}

float VinylControl::getSpeed()
{
    return dVinylScratch;
}
