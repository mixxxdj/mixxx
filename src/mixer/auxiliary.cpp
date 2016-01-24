#include "mixer/auxiliary.h"

#include "engine/engineaux.h"
#include "engine/enginemaster.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"

Auxiliary::Auxiliary(QObject* pParent, const QString& group, int index,
                     std::shared_ptr<SoundManager> pSoundManager,
                     std::shared_ptr<EngineMaster> pEngine,
                     std::shared_ptr<EffectsManager> pEffectsManager)
        : BasePlayer(pParent, group) {
    ChannelHandleAndGroup channelGroup = pEngine->registerChannelGroup(group);
    EngineAux* pAuxiliary = new EngineAux(channelGroup, pEffectsManager.get());
    pEngine->addChannel(pAuxiliary);
    AudioInput auxInput = AudioInput(AudioPath::AUXILIARY, 0, 2, index);
    pSoundManager->registerInput(auxInput, pAuxiliary);
}

Auxiliary::~Auxiliary() {
}
