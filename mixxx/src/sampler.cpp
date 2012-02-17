#include "sampler.h"

Sampler::Sampler(QObject* pParent,
                 ConfigObject<ConfigValue>* pConfig,
                 EngineMaster* pMixingEngine,
                 EngineChannel::ChannelOrientation defaultOrientation,
                 QString group)
        : BaseTrackPlayer(pParent, pConfig, pMixingEngine, defaultOrientation, NULL, group) {

}

Sampler::~Sampler()
{
}
