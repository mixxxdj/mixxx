

#include <QtDebug>
#include "vinylcontrolproxy.h"
#include "vinylcontrolxwax.h"
#include "controlobjectthreadmain.h"


//IVinylControl *VinylControlProxy::m_pVinylControl = 0;

VinylControlProxy::VinylControlProxy(ConfigObject<ConfigValue> * pConfig, const char * _group) : VinylControl(pConfig, _group)
{
    QList<QString> xwax_timecodes;
    QList<QString> scratchlib_timecodes;

    xwax_timecodes.push_back(MIXXX_VINYL_SERATOCV02VINYLSIDEA);
    xwax_timecodes.push_back(MIXXX_VINYL_SERATOCV02VINYLSIDEB);
    xwax_timecodes.push_back(MIXXX_VINYL_SERATOCD);
    xwax_timecodes.push_back(MIXXX_VINYL_TRAKTORSCRATCHSIDEA);
    xwax_timecodes.push_back(MIXXX_VINYL_TRAKTORSCRATCHSIDEB);

    //Figure out which type of timecoded vinyl we're using.
    strVinylType = m_pConfig->getValueString(ConfigKey(_group,"vinylcontrol_vinyl_type"));

    //Create the VinylControl object that matches the type of vinyl selected in the prefs...
    if (xwax_timecodes.contains(strVinylType))
    {
        m_pVinylControl = new VinylControlXwax(pConfig, _group);
    }
    else
    {
        qDebug() << "VinylControlProxy: Unknown vinyl type " << strVinylType;
        qDebug() << "Defaulting to Serato...";
        strVinylType = MIXXX_VINYL_SERATOCV02VINYLSIDEA;
        pConfig->set(ConfigKey(_group,"vinylcontrol_vinyl_type"), ConfigValue(MIXXX_VINYL_SERATOCV02VINYLSIDEA));
        m_pVinylControl = new VinylControlXwax(pConfig, _group);
    }
}


VinylControlProxy::~VinylControlProxy()
{
    if (m_pVinylControl)
    {
        delete m_pVinylControl;
        m_pVinylControl = NULL;
    }
}

bool VinylControlProxy::isEnabled()
{
    return bIsEnabled;
}

void VinylControlProxy::ToggleVinylControl(bool enable)
{
    if (m_pVinylControl)
        m_pVinylControl->ToggleVinylControl(enable);
    enabled->slotSet(enable);
}

/*
   void VinylControlProxy::syncPitch(double pitch)
   {

   }

   void VinylControlProxy::syncPosition()
   {

   } */

void VinylControlProxy::AnalyseSamples(const short * samples, size_t size)
{
    if (m_pVinylControl && samples)
        m_pVinylControl->AnalyseSamples(samples, size);
}

void VinylControlProxy::run()
{
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("VinylControlProxy %1").arg(++id));
}

float VinylControlProxy::getSpeed()
{
    if (m_pVinylControl)
        return m_pVinylControl->getSpeed();
    else
        return 0.0f;
}

/** Returns some sort of indication of the vinyl's signal quality.
    Range of m_fTimecodeQuality should be 0.0 to 1.0 */
float VinylControlProxy::getTimecodeQuality()
{
    if (m_pVinylControl)
        return m_pVinylControl->getTimecodeQuality();
    else
        return 0.0f;
}

unsigned char* VinylControlProxy::getScopeBytemap()
{
    if (m_pVinylControl)
        return m_pVinylControl->getScopeBytemap();
    else
        return NULL;
}

float VinylControlProxy::getAngle()
{
    if (m_pVinylControl)
        return m_pVinylControl->getAngle();
    else
        return -1.0;
}

void VinylControlProxy::receiveBuffer(AudioInput input, const short* pBuffer,
        unsigned int iNumFrames) {
    if (input.getType() != AudioPath::VINYLCONTROL ||
        QString("[Channel%1]").arg(input.getIndex()+1) != this->group) {
        qDebug() << "WARNING: vinyl control proxy got a buffer for an "
            "AudioInput it doesn't own";
    }
    AnalyseSamples(pBuffer, iNumFrames);
}
