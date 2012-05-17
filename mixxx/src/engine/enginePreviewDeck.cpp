#include "controlpushbutton.h"
#include "enginebuffer.h"
#include "enginePreviewDeck.h"
#include "engineclipping.h"
#include "enginepregain.h"
#include "enginevumeter.h"

EnginePreviewDeck::EnginePreviewDeck(const char* group,
                             ConfigObject<ConfigValue>* pConfig,
                             EngineChannel::ChannelOrientation defaultOrientation)
        : EngineChannel(group, defaultOrientation),
          m_pConfig(pConfig) {
    m_pClipping = new EngineClipping(group);
    m_pBuffer = new EngineBuffer(group, pConfig);
    m_pVUMeter = new EngineVuMeter(group);
}

EnginePreviewDeck::~EnginePreviewDeck() {
    delete m_pBuffer;
    delete m_pClipping;
    delete m_pVUMeter;
}

void EnginePreviewDeck::process(const CSAMPLE*, const CSAMPLE * pOut, const int iBufferSize) {
    // Process the raw audio
    m_pBuffer->process(0, pOut, iBufferSize);
    // Apply clipping
    m_pClipping->process(pOut, pOut, iBufferSize);
    // Update VU meter
    m_pVUMeter->process(pOut, pOut, iBufferSize);
}

EngineBuffer* EnginePreviewDeck::getEngineBuffer() {
    return m_pBuffer;
}

bool EnginePreviewDeck::isActive() {
    return m_pBuffer->isTrackLoaded();
}

bool EnginePreviewDeck::isPFL() {
    return true;
}

bool EnginePreviewDeck::isMaster() {
    return false;
}
