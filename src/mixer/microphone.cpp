#include "mixer/microphone.h"

#include "control/controlproxy.h"
#include "engine/channels/enginemicrophone.h"
#include "engine/enginemaster.h"
#include "moc_microphone.cpp"
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
    AudioInput micInput = AudioInput(AudioPath::MICROPHONE, 0, 2, index);
    pSoundManager->registerInput(micInput, pMicrophone);

    m_pInputConfigured = make_parented<ControlProxy>(group, "input_configured", this);
    m_pTalkoverEnabled = make_parented<ControlProxy>(group, "talkover", this);
    m_pTalkoverEnabled->connectValueChanged(this, &Microphone::slotTalkoverEnabled);
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
        emit noMicrophoneInputConfigured();
    }
}
