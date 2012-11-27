#include "sampler.h"

#include "controlobjectthreadmain.h"
#include "controlobject.h"

Sampler::Sampler(QObject* pParent,
                 ConfigObject<ConfigValue>* pConfig,
                 EngineMaster* pMixingEngine,
                 EngineChannel::ChannelOrientation defaultOrientation,
                 QString group)
        : BaseTrackPlayer(pParent, pConfig, pMixingEngine, defaultOrientation,
                          NULL, group, true, false) {
}

Sampler::~Sampler()
{
}
