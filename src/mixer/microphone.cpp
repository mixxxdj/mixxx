#include "mixer/microphone.h"

#include "controlobjectslave.h"
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

    m_pInputConfigured.reset(new ControlObjectSlave(group, "input_configured", this));
    m_pTalkoverEnabled.reset(new ControlObjectSlave(group, "talkover", this));
    m_pTalkoverEnabled->connectValueChanged(SLOT(slotTalkoverEnabled(double)));
}

Microphone::~Microphone() {
}

void Microphone::slotTalkoverEnabled(double v) {
    bool configured = m_pInputConfigured->toBool();
    bool talkover = v > 0.0;

    // Warn the user if they try to enable talkover on a microphone with no
    // configured input.
    if (!configured && talkover) {
        m_pTalkoverEnabled->set(0.0);
        emit(noMicrophoneInputConfigured());
    }
}
