#include "mixer/auxiliary.h"

#include "engine/engineaux.h"
#include "engine/enginemaster.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"

Auxiliary::Auxiliary(QObject* pParent, const QString& group, int index,
                     SoundManager* pSoundManager, EngineMaster* pEngine,
                     EffectsManager* pEffectsManager)
        : BasePlayer(pParent, group) {
    ChannelHandleAndGroup channelGroup = pEngine->registerChannelGroup(group);
    EngineAux* pAuxiliary = new EngineAux(channelGroup, pEffectsManager);
    pEngine->addChannel(pAuxiliary);
    AudioInput auxInput = AudioInput(AudioPath::AUXILIARY, 0, 0, index);
    pSoundManager->registerInput(auxInput, pAuxiliary);
}

Auxiliary::~Auxiliary() {
}
