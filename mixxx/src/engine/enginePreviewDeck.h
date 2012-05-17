#ifndef ENGINELIBPREVIEWPLAYER_H
#define ENGINELIBPREVIEWPLAYER_H

#include "engine/engineobject.h"
#include "engine/enginechannel.h"
#include "configobject.h"

class EngineBuffer;
class EngineClipping;
class EngineVuMeter;

class EnginePreviewDeck : public EngineChannel {
    Q_OBJECT
  public:
    EnginePreviewDeck(const char *group, ConfigObject<ConfigValue>* pConfig,
               EngineChannel::ChannelOrientation defaultOrientation = CENTER);
    virtual ~EnginePreviewDeck();

    virtual void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

    // TODO(XXX) This hack needs to be removed.
    virtual EngineBuffer* getEngineBuffer();

    virtual bool isActive();
    virtual bool isPFL();
    virtual bool isMaster();
  private:
    ConfigObject<ConfigValue>* m_pConfig;
    EngineBuffer* m_pBuffer;
    EngineClipping* m_pClipping;
    EngineVuMeter* m_pVUMeter;
};

#endif
