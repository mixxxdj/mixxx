// bpmcontrol.cpp
// Created 7/5/2009 by RJ Ryan (rryan@mit.edu)

#include "controlobject.h"
#include "controlpushbutton.h"

#include "engine/enginebuffer.h"
#include "engine/bpmcontrol.h"

const int minBpm = 30;
const int maxInterval = (int)(1000.*(60./(CSAMPLE)minBpm));
const int filterLength = 5;

BpmControl::BpmControl(const char* _group,
                       ConfigObject<ConfigValue>* _config) :
        EngineControl(_group, _config),
        m_tapFilter(this, filterLength, maxInterval) {

    m_pRateSlider = ControlObject::getControl(ConfigKey(_group, "rate"));
    m_pRateRange = ControlObject::getControl(ConfigKey(_group, "rateRange"));
    m_pRateDir = ControlObject::getControl(ConfigKey(_group, "rate_dir"));

    m_pFileBpm = new ControlObject(ConfigKey(_group, "file_bpm"));
    connect(m_pFileBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotFileBpmChanged(double)),
            Qt::DirectConnection);

    m_pEngineBpm = new ControlObject(ConfigKey(_group, "bpm"));
    connect(m_pEngineBpm, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetEngineBpm(double)),
            Qt::DirectConnection);

    m_pButtonTap = new ControlPushButton(ConfigKey(_group, "bpm_tap"));
    connect(m_pButtonTap, SIGNAL(valueChanged(double)),
            this, SLOT(slotBpmTap(double)),
            Qt::DirectConnection);

    // Beat sync (scale buffer tempo relative to tempo of other buffer)
    m_pButtonSync = new ControlPushButton(ConfigKey(_group, "beatsync"));
    connect(m_pButtonSync, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlBeatSync(double)),
            Qt::DirectConnection);

    connect(&m_tapFilter, SIGNAL(tapped(double,int)),
            this, SLOT(slotTapFilter(double,int)),
            Qt::DirectConnection);
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

    if (filebpm != 0.0) {
        double newRate = bpm / filebpm - 1.0f;
        newRate = math_max(-1.0f, math_min(1.0f, newRate));
        m_pRateSlider->set(newRate);
    }
}

void BpmControl::slotBpmTap(double v) {
    if (v > 0) {
        m_tapFilter.tap();
    }
}

void BpmControl::slotTapFilter(double averageLength, int numSamples) {
    // averageLength is the average interval in milliseconds tapped over
    // numSamples samples.  Have to convert to BPM now:

    if (averageLength <= 0)
        return;

    if (numSamples < 4)
        return;

    // (60 seconds per minute) * (1000 milliseconds per second) / (X millis per
    // beat) = Y beats/minute
    double averageBpm = 60.0 * 1000.0 / averageLength;
    slotSetEngineBpm(averageBpm);
}

void BpmControl::slotControlBeatSync(double)
{
    EngineBuffer* pOtherEngineBuffer = getOtherEngineBuffer();

    if(!pOtherEngineBuffer)
        return;

    double fOtherBpm = pOtherEngineBuffer->getBpm();
    double fThisBpm  = m_pEngineBpm->get();
    double fRateScale;

    if (fOtherBpm>0. && fThisBpm>0.)
    {
        float fOtherRate = pOtherEngineBuffer->getRate();
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

