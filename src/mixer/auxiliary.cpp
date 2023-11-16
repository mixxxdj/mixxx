#include "mixer/auxiliary.h"

#include "audio/types.h"
#include "control/controlproxy.h"
#include "engine/channels/engineaux.h"
#include "engine/enginemixer.h"
#include "moc_auxiliary.cpp"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"

Auxiliary::Auxiliary(PlayerManager* pParent,
        const QString& group,
        int index,
        SoundManager* pSoundManager,
        EngineMixer* pEngine,
        EffectsManager* pEffectsManager)
        : BasePlayer(pParent, group) {
    ChannelHandleAndGroup channelGroup = pEngine->registerChannelGroup(group);
    EngineAux* pAuxiliary = new EngineAux(channelGroup, pEffectsManager);
    pEngine->addChannel(pAuxiliary);
    AudioInput auxInput = AudioInput(AudioPathType::Auxiliary,
            0,
            mixxx::audio::ChannelCount::stereo(),
            index);
    pSoundManager->registerInput(auxInput, pAuxiliary);

    m_pInputConfigured = make_parented<ControlProxy>(group, "input_configured", this);
    m_pAuxMainMixEnabled = make_parented<ControlProxy>(group, "main_mix", this);
    m_pAuxMainMixEnabled->connectValueChanged(this, &Auxiliary::slotAuxMainMixEnabled);
}

Auxiliary::~Auxiliary() {
}

void Auxiliary::slotAuxMainMixEnabled(double v) {
    bool configured = m_pInputConfigured->toBool();
    bool auxMainMixEnable = v > 0.0;

    // Warn the user if they try to enable main on a auxiliary with no
    // configured input.
    if (!configured && auxMainMixEnable) {
        m_pAuxMainMixEnabled->set(0.0);
        emit noAuxiliaryInputConfigured();
    }
}
