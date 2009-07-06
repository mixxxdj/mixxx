// bpmcontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include "controlobject.h"
#include "controlbeat.h"
#include "controlpushbutton.h"

#include "engine/enginebuffer.h"
#include "engine/bpmcontrol.h"

BpmControl::BpmControl(const char* _group,
                       const ConfigObject<ConfigValue>* _config) :
    EngineControl(_group, _config),
    m_pOtherEngineBuffer(NULL) {

    m_pRateSlider = ControlObject::getControl(ConfigKey(_group, "rate"));
    m_pRateRange = ControlObject::getControl(ConfigKey(_group, "rateRange"));
    m_pRateDir = ControlObject::getControl(ConfigKey(_group, "rate_dir"));

    m_pFileBpm = new ControlObject(ConfigKey(_group, "file_bpm"));
    connect(m_pFileBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotFileBpmChanged(double)));

    m_pEngineBpm = new ControlBeat(ConfigKey(_group, "bpm"), true);
    connect(m_pEngineBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetEngineBpm(double)));

    // Beat sync (scale buffer tempo relative to tempo of other buffer)
    m_pButtonSync = new ControlPushButton(ConfigKey(_group, "beatsync"));
    connect(m_pButtonSync, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlBeatSync(double)));

}

BpmControl::~BpmControl() {
    delete m_pEngineBpm;
    delete m_pFileBpm;
    delete m_pButtonSync;
}

double BpmControl::getBpm() {
    return m_pEngineBpm->get();
}

void BpmControl::slotFileBpmChanged(double bpm) {
    m_pEngineBpm->set(bpm);
}

void BpmControl::slotSetEngineBpm(double bpm) {
    double filebpm = m_pFileBpm->get();

    if (filebpm!=0.)
        m_pRateSlider->set(bpm/filebpm-1.);
}

void BpmControl::slotControlBeatSync(double)
{

    if(!m_pOtherEngineBuffer)
        return;
    
    double fOtherBpm = m_pOtherEngineBuffer->getBpm();
    double fThisBpm  = m_pEngineBpm->get();
    double fRateScale;

    if (fOtherBpm>0. && fThisBpm>0.)
    {
        float fOtherRate = m_pOtherEngineBuffer->getRate();
        float fBpmDelta = fabs(fThisBpm-fOtherBpm);

        // Test if this buffers bpm is the double of the other one, and find
        // rate scale:
        if (fabs(fThisBpm*2.-fOtherBpm) < fBpmDelta)
            fRateScale = fOtherBpm/(2*fThisBpm) * (1.+fOtherRate);
        else if ( fabs(fThisBpm-2.*fOtherBpm) < fBpmDelta)
            fRateScale = 2.*fOtherBpm/fThisBpm * (1.+fOtherRate);
        else
            fRateScale = (fOtherBpm*(1.+fOtherRate))/fThisBpm;

        // Ensure the rate is within resonable boundaries
        if (fRateScale<2. && fRateScale>0.5)
        {
            // Adjust the rate:
            fRateScale = (fRateScale-1.)/m_pRateRange->get();
            m_pRateSlider->set(fRateScale * m_pRateDir->get());

            // Adjust the phase:
            // (removed, see older version for this info)
        }
    }

}
