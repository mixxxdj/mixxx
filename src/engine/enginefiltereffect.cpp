#include "enginefiltereffect.h"

#include "sampleutil.h"
#include "controlobject.h"
#include "enginefilterbutterworth8.h"
#include "controlpushbutton.h"
#include "controlpotmeter.h"

EngineFilterEffect::EngineFilterEffect(const char* group) {
    potmeterDepth = ControlObject::getControl(ConfigKey(group, "filterDepth"));
    if (potmeterDepth == NULL) {
        potmeterDepth = new ControlPotmeter(ConfigKey(group, "filterDepth"),
                                            -1., 1.);
    }

    filterEnable = new ControlPushButton(ConfigKey(group, "filter"));
    filterEnable->setButtonMode(ControlPushButton::TOGGLE);

    // TODO(XXX) 44100 should be changed to real sample rate
    // https://bugs.launchpad.net/mixxx/+bug/1208816
    m_pLowFilter = new EngineFilterButterworth8Low(44100, 20);
    m_pBandpassFilter = new EngineFilterButterworth8Band(44100, 20, 200);
    m_pHighFilter = new EngineFilterButterworth8High(44100, 20);

    m_pCrossfade_buffer = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_pBandpass_buffer = SampleUtil::alloc(MAX_BUFFER_LEN);

    old_depth = 0.0f;
}

EngineFilterEffect::~EngineFilterEffect() {
    delete m_pLowFilter;
    delete m_pBandpassFilter;
    delete m_pHighFilter;

    delete potmeterDepth;
    delete filterEnable;

    SampleUtil::free(m_pCrossfade_buffer);
    SampleUtil::free(m_pBandpass_buffer);
}

void EngineFilterEffect::applyFilters(const CSAMPLE* pIn, CSAMPLE* pOut,
                                      const int iBufferSize, float depth) {
    // Gain of bandpass filter
    float bandpass_gain = 0.3f;

    if (depth < 0.0f) {
        m_pLowFilter->process(pIn, pOut, iBufferSize);
        m_pBandpassFilter->process(pIn, m_pBandpass_buffer, iBufferSize);
    } else {
        m_pHighFilter->process(pIn, pOut, iBufferSize);
        m_pBandpassFilter->process(pIn, m_pBandpass_buffer, iBufferSize);
    }

    SampleUtil::addWithGain(pOut, m_pBandpass_buffer, bandpass_gain, iBufferSize);
}

void EngineFilterEffect::process(const CSAMPLE* pIn, const CSAMPLE* pOut,
                                 const int iBufferSize) {
    CSAMPLE* pOutput = (CSAMPLE*)pOut;
    float depth = potmeterDepth->get();

    if (filterEnable->get() == 0.0f) {
        depth = 0.0;
    }

    float freq, freq2;
    // Length of bandpass filter
    float bandpass_size = 0.01f;

    if (depth != old_depth) {
        if (old_depth == 0.0f) {
            SampleUtil::copyWithGain(m_pCrossfade_buffer, pIn, 1.0f, iBufferSize);
        } else if (old_depth == -1.0f || old_depth == 1.0f) {
            SampleUtil::copyWithGain(m_pCrossfade_buffer, pIn, 0.0f, iBufferSize);
        } else {
            applyFilters(pIn, m_pCrossfade_buffer, iBufferSize, old_depth);
        }
        if (depth < 0.0f) {
            // Lowpass + bandpass
            // Freq from 2^5=32Hz to 2^(5+9)=16384
            freq = pow(2.0, 5.0f + (depth + 1.0f) * 9.0f);
            freq2 = pow(2.0, 5.0f + (depth + 1.0f + bandpass_size) * 9.0f);
            m_pLowFilter->setFrequencyCorners(freq2);
            m_pBandpassFilter->setFrequencyCorners(freq, freq2);
        } else if (depth > 0.0f) {
            // Highpass + bandpass
            freq = pow(2.0, 5.0f + depth * 9.0f);
            freq2 = pow(2.0, 5.0f + (depth + bandpass_size) * 9.0f);
            m_pHighFilter->setFrequencyCorners(freq);
            m_pBandpassFilter->setFrequencyCorners(freq, freq2);
        }
    }

    if (depth == 0.0f) {
        SampleUtil::copyWithGain(pOutput, pIn, 1.0f, iBufferSize);
    } else if (depth == -1.0f || depth == 1.0f) {
        SampleUtil::copyWithGain(pOutput, pIn, 0.0f, iBufferSize);
    } else {
        applyFilters(pIn, pOutput, iBufferSize, depth);
    }

    if (depth != old_depth) {
        SampleUtil::linearCrossfadeBuffers(pOutput, m_pCrossfade_buffer,
                                           pOutput, iBufferSize);
    }

    old_depth = depth;
}

