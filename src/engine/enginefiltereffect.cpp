#include "engine/enginefiltereffect.h"

#include "sampleutil.h"
#include "controlobject.h"
#include "engine/enginefilterbutterworth8.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"

EngineFilterEffect::EngineFilterEffect(const char* group) {
    m_pPotmeterDepth = new ControlPotmeter(ConfigKey(group, "filterDepth"), -1., 1.);
    m_pFilterEnable = new ControlPushButton(ConfigKey(group, "filter"));
    m_pFilterEnable->setButtonMode(ControlPushButton::TOGGLE);

    // TODO(XXX) 44100 should be changed to real sample rate
    // https://bugs.launchpad.net/mixxx/+bug/1208816
    m_pLowFilter = new EngineFilterButterworth8Low(44100, 20);
    m_pBandpassFilter = new EngineFilterButterworth8Band(44100, 20, 200);
    m_pHighFilter = new EngineFilterButterworth8High(44100, 20);

    m_pCrossfade_buffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pBandpass_buffer = SampleUtil::alloc(MAX_BUFFER_LEN);

    m_old_depth = 0.0;
}

EngineFilterEffect::~EngineFilterEffect() {
    delete m_pLowFilter;
    delete m_pBandpassFilter;
    delete m_pHighFilter;

    delete m_pPotmeterDepth;
    delete m_pFilterEnable;

    SampleUtil::free(m_pCrossfade_buffer);
    SampleUtil::free(m_pBandpass_buffer);
}

void EngineFilterEffect::applyFilters(const CSAMPLE* pIn, CSAMPLE* pOut,
                                      const int iBufferSize, double depth) {
    // Gain of bandpass filter
    double bandpass_gain = 0.3;

    if (depth < 0.0) {
        m_pLowFilter->process(pIn, pOut, iBufferSize);
        m_pBandpassFilter->process(pIn, m_pBandpass_buffer, iBufferSize);
    } else {
        m_pHighFilter->process(pIn, pOut, iBufferSize);
        m_pBandpassFilter->process(pIn, m_pBandpass_buffer, iBufferSize);
    }

    SampleUtil::addWithGain(pOut, m_pBandpass_buffer, bandpass_gain, iBufferSize);
}

void EngineFilterEffect::process(const CSAMPLE* pIn, CSAMPLE* pOutput,
                                 const int iBufferSize) {
    double depth = m_pPotmeterDepth->get();

    if (m_pFilterEnable->get() == 0.0) {
        depth = 0.0;
    }

    double freq, freq2;
    // Length of bandpass filter
    double bandpass_size = 0.01;

    if (depth != m_old_depth) {
        if (m_old_depth == 0.0) {
            SampleUtil::copyWithGain(m_pCrossfade_buffer, pIn, 1.0, iBufferSize);
        } else if (m_old_depth == -1.0 || m_old_depth == 1.0) {
            SampleUtil::copyWithGain(m_pCrossfade_buffer, pIn, 0.0, iBufferSize);
        } else {
            applyFilters(pIn, m_pCrossfade_buffer, iBufferSize, m_old_depth);
        }
        if (depth < 0.0) {
            // Lowpass + bandpass
            // Freq from 2^5=32Hz to 2^(5+9)=16384
            freq = pow(2.0, 5.0 + (depth + 1.0) * 9.0);
            freq2 = pow(2.0, 5.0 + (depth + 1.0 + bandpass_size) * 9.0);
            m_pLowFilter->setFrequencyCorners(freq2);
            m_pBandpassFilter->setFrequencyCorners(freq, freq2);
        } else if (depth > 0.0) {
            // Highpass + bandpass
            freq = pow(2.0, 5.0 + depth * 9.0);
            freq2 = pow(2.0, 5.0 + (depth + bandpass_size) * 9.0);
            m_pHighFilter->setFrequencyCorners(freq);
            m_pBandpassFilter->setFrequencyCorners(freq, freq2);
        }
    }

    if (depth == 0.0) {
        SampleUtil::copyWithGain(pOutput, pIn, 1.0, iBufferSize);
    } else if (depth == -1.0 || depth == 1.0) {
        SampleUtil::copyWithGain(pOutput, pIn, 0.0, iBufferSize);
    } else {
        applyFilters(pIn, pOutput, iBufferSize, depth);
    }

    if (depth != m_old_depth) {
        SampleUtil::linearCrossfadeBuffers(pOutput, m_pCrossfade_buffer,
                                           pOutput, iBufferSize);
    }

    m_old_depth = depth;
}
