

#include <QtDebug>
#include "vinylcontrolproxy.h"
#include "vinylcontrolscratchlib.h"
#include "vinylcontrolxwax.h"


//IVinylControl *VinylControlProxy::m_pVinylControl = 0;

VinylControlProxy::VinylControlProxy(ConfigObject<ConfigValue> *pConfig, const char *_group) : VinylControl(pConfig, _group)
{
	QList<QString> xwax_timecodes;
	QList<QString> scratchlib_timecodes;
	
	xwax_timecodes.push_back(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    xwax_timecodes.push_back(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
	xwax_timecodes.push_back(MIXXX_VINYL_SERATOCD);
	xwax_timecodes.push_back(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
	xwax_timecodes.push_back(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);
	scratchlib_timecodes.push_back(MIXXX_VINYL_FINALSCRATCH);
	scratchlib_timecodes.push_back(MIXXX_VINYL_MIXVIBESDVSCD);	
	
	//Figure out which type of timecoded vinyl we're using.
	strVinylType = m_pConfig->getValueString(ConfigKey("[VinylControl]","strVinylType"));
		
	//Create the VinylControl object that matches the type of vinyl selected in the prefs...
	if (scratchlib_timecodes.contains(strVinylType))
	{
	    m_pVinylControl = new VinylControlScratchlib(pConfig, _group);
	}    
	else if (xwax_timecodes.contains(strVinylType))
	{
	    m_pVinylControl = new VinylControlXwax(pConfig, _group);
    }
    else
    {
        qDebug() << "VinylControlProxy: Unknown vinyl type " << strVinylType;
        qDebug() << "Defaulting to Serato...";
        strVinylType = MIXXX_VINYL_SERATOCV02VINYLSIDEA;
        pConfig->set(ConfigKey("[VinylControl]","strVinylType"), ConfigValue(MIXXX_VINYL_SERATOCV02VINYLSIDEA));
        m_pVinylControl = new VinylControlXwax(pConfig, _group);    
    }
}


VinylControlProxy::~VinylControlProxy()
{

    if (m_pVinylControl)
        delete m_pVinylControl;


}

bool VinylControlProxy::isEnabled()
{ 
    return bIsRunning;
}

void VinylControlProxy::ToggleVinylControl(bool enable) 
{
    if (m_pVinylControl)
        m_pVinylControl->ToggleVinylControl(enable);
}

/*
void VinylControlProxy::syncPitch(double pitch)
{
    
}

void VinylControlProxy::syncPosition()
{

} */

void VinylControlProxy::AnalyseSamples(short* samples, size_t size)
{
    if (m_pVinylControl && samples)
        m_pVinylControl->AnalyseSamples(samples, size);
}

void VinylControlProxy::run()
{
}

float VinylControlProxy::getSpeed()
{
    if (m_pVinylControl)
        return m_pVinylControl->getSpeed();
    else 
        return 0.0f;
}
