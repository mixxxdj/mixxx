#include "mixer/microphone.h"

#include "engine/enginemaster.h"
#include "engine/enginemicrophone.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"

Microphone::Microphone(QObject* pParent, const QString& group, int index,
                       SoundManager* pSoundManager, EngineMaster* pEngine,
                       EffectsManager* pEffectsManager)
        : BasePlayer(pParent, group) {
    ChannelHandleAndGroup channelGroup = pEngine->registerChannelGroup(group);
    EngineMicrophone* pMicrophone =
            new EngineMicrophone(channelGroup, pEffectsManager);
    pEngine->addChannel(pMicrophone);
    AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 0, index);
    pSoundManager->registerInput(micInput, pMicrophone);
}

Microphone::~Microphone() {
}
