#include "mixer/microphone.h"

#include "audio/types.h"
#include "control/controlproxy.h"
#include "engine/channels/enginemicrophone.h"
#include "engine/enginemixer.h"
#include "moc_microphone.cpp"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"

Microphone::Microphone(PlayerManager* pParent,
        const QString& group,
        int index,
        SoundManager* pSoundManager,
        EngineMixer* pEngine,
        EffectsManager* pEffectsManager)
        : BasePlayer(pParent, group) {
    ChannelHandleAndGroup channelGroup = pEngine->registerChannelGroup(group);
    auto pMicrophone = std::make_unique<EngineMicrophone>(channelGroup, pEffectsManager);
    AudioInput micInput = AudioInput(AudioPathType::Microphone,
            0,
            mixxx::audio::ChannelCount::stereo(),
            index);
    pSoundManager->registerInput(micInput, pMicrophone.get());
    pEngine->addChannel(std::move(pMicrophone));

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
