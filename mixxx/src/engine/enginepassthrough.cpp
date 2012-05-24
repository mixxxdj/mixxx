// enginepassthrough.cpp
// created 4/8/2011 by Bill Good (bkgood@gmail.com)
// shameless stolen from enginemicrophone.cpp (from RJ)

#include <QtDebug>

#include "engine/enginepassthrough.h"

#include "configobject.h"
#include "sampleutil.h"

EnginePassthrough::EnginePassthrough(const char* pGroup, const char* pDeckGroup)
        : EngineChannel(pGroup, EngineChannel::CENTER),
          m_clipping(pGroup),
          m_vuMeter(pGroup),
          m_pEnabled(new ControlObject(ConfigKey(pGroup, "passthrough_enabled"))),
          m_pPassing(new ControlPushButton(ConfigKey(pGroup, "passthrough_passing"))),
          m_pConversionBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          // Need a +1 here because the CircularBuffer only allows its size-1
          // items to be held at once (it keeps a blank spot open persistently)
          m_sampleBuffer(MAX_BUFFER_LEN+1) {
    m_pPassing->setButtonMode(ControlPushButton::TOGGLE);

    m_pPregain = new EnginePregain(pGroup);
    m_pFilter = new EngineFilterBlock(pGroup);
    m_pFlanger = new EngineFlanger(pGroup);
    m_pClipping = new EngineClipping(pGroup);
    m_pVUMeter = new EngineVuMeter(pGroup);

    // Connect pregain controls
    ControlObject::connectControls(ConfigKey(pDeckGroup, "pregain"), ConfigKey(pGroup, "pregain"));
    ControlObject::connectControls(ConfigKey(pDeckGroup, "replaygain"), ConfigKey(pGroup, "replaygain"));
    ControlObject::connectControls(ConfigKey(pDeckGroup, "total_gain"), ConfigKey(pGroup, "total_gain"));

    // Connect filter controls
    ControlObject::connectControls(ConfigKey(pDeckGroup, "filterLow"), ConfigKey(pGroup, "filterLow"));
    ControlObject::connectControls(ConfigKey(pDeckGroup, "filterLowKill"), ConfigKey(pGroup, "filterLowKill"));
    ControlObject::connectControls(ConfigKey(pDeckGroup, "filterMid"), ConfigKey(pGroup, "filterMid"));
    ControlObject::connectControls(ConfigKey(pDeckGroup, "filterMidKill"), ConfigKey(pGroup, "filterMidKill"));
    ControlObject::connectControls(ConfigKey(pDeckGroup, "filterHigh"), ConfigKey(pGroup, "filterHigh"));
    ControlObject::connectControls(ConfigKey(pDeckGroup, "filterHighKill"), ConfigKey(pGroup, "filterHighKill"));

    // Connect flanger controls
    ControlObject::connectControls(ConfigKey(pDeckGroup, "flanger"), ConfigKey(pGroup, "flanger"));

    // Connect clipper controls
    ControlObject::connectControls(ConfigKey(pDeckGroup, "PeakIndicator"), ConfigKey(pGroup, "PeakIndicator"));

    // Connect vu meter controls
    ControlObject::connectControls(ConfigKey(pDeckGroup, "VuMeter"), ConfigKey(pGroup, "VuMeter"));
    ControlObject::connectControls(ConfigKey(pDeckGroup, "VuMeterL"), ConfigKey(pGroup, "VuMeterL"));
    ControlObject::connectControls(ConfigKey(pDeckGroup, "VuMeterR"), ConfigKey(pGroup, "VuMeterR"));
}

EnginePassthrough::~EnginePassthrough() {
    qDebug() << "~EnginePassthrough()";
    SampleUtil::free(m_pConversionBuffer);
    delete m_pEnabled;
    delete m_pPassing;

    delete m_pClipping;
    delete m_pVUMeter;
    delete m_pFlanger;
    delete m_pFilter;
    delete m_pPregain;
}

bool EnginePassthrough::isActive() {
    bool enabled = m_pEnabled->get() > 0.0;
    return enabled && !m_sampleBuffer.isEmpty();
}

bool EnginePassthrough::isPFL() {
    return true;
}

bool EnginePassthrough::isMaster() {
    return true;
}

void EnginePassthrough::onInputConnected(AudioInput input) {
    if (input.getType() != AudioPath::VINYLCONTROL) {
        // This is an error!
        qDebug() << "WARNING: EnginePassthrough connected to AudioInput for a non-vinylcontrol type!";
        return;
    }
    m_sampleBuffer.clear();
    m_pEnabled->set(1.0f);
}

void EnginePassthrough::onInputDisconnected(AudioInput input) {
    if (input.getType() != AudioPath::VINYLCONTROL) {
        // This is an error!
        qDebug() << "WARNING: EnginePassthrough connected to AudioInput for a non-vinylcontrol type!";
        return;
    }

    m_sampleBuffer.clear();
    m_pEnabled->set(0.0f);
}

void EnginePassthrough::receiveBuffer(AudioInput input, const short* pBuffer, unsigned int nFrames) {
    if (input.getType() != AudioPath::VINYLCONTROL) {
        // This is an error!
        qDebug() << "WARNING: EnginePassthrough receieved an AudioInput for a non-vinylcontrol type!";
        return;
    }

    // Use the conversion buffer to both convert from short and double into
    // stereo.

    // Check that the number of mono samples doesn't exceed MAX_BUFFER_LEN/2
    // because thats our conversion buffer size.
    if (nFrames > MAX_BUFFER_LEN / 2) {
        qDebug() << "WARNING: Dropping passthrough samples because the input buffer is too large.";
        nFrames = MAX_BUFFER_LEN / 2;
    }

    // There isn't a suitable SampleUtil method that can do mono->stereo and
    // short->float in one pass.
    // SampleUtil::convert(m_pConversionBuffer, pBuffer, iNumSamples);
    SampleUtil::convert(m_pConversionBuffer, pBuffer, nFrames*2);

    // TODO(rryan) (or bkgood?) do we need to verify the input is the one we asked for? Oh well.
    unsigned int samplesWritten = m_sampleBuffer.write(m_pConversionBuffer, nFrames*2);
    if (samplesWritten < nFrames*2) {
        // Buffer overflow. We aren't processing samples fast enough. This
        // shouldn't happen since the deck spits out samples just as fast as they
        // come in, right?
        Q_ASSERT(false);
    }
}

void EnginePassthrough::process(const CSAMPLE* pInput, const CSAMPLE* pOutput, const int iBufferSize) {
    CSAMPLE* pOut = const_cast<CSAMPLE*>(pOutput);
    Q_UNUSED(pInput);

    // If passthrough is enabled, then read into the output buffer. Otherwise,
    // Note: Remove true segment from following conditional to allow toggling passthrough	
    if(m_pPassing->get() > 0.0f) {
        int samplesRead = m_sampleBuffer.read(pOut, iBufferSize);
        if (samplesRead < iBufferSize) {
            // Buffer underflow. There aren't getting samples fast enough. This
            // shouldn't happen since PortAudio should feed us samples just as fast
            // as we consume them, right?
            Q_ASSERT(false);
        }
    } else {
        SampleUtil::applyGain(pOut, 0.0, iBufferSize);
        m_sampleBuffer.skip(iBufferSize);
    }

    // Apply clipping
    m_clipping.process(pOut, pOut, iBufferSize);
    // Update VU meter
    m_vuMeter.process(pOut, pOut, iBufferSize);

    // Apply pregain
    m_pPregain->process(pOut, pOut, iBufferSize);
    // Filter the channel with EQs
    m_pFilter->process(pOut, pOut, iBufferSize);
    // Apply flanger
    m_pFlanger->process(pOut, pOut, iBufferSize);

    // Apply clipping
    m_pClipping->process(pOut, pOut, iBufferSize);
    // Update VU meter
    m_pVUMeter->process(pOut, pOut, iBufferSize);
}
