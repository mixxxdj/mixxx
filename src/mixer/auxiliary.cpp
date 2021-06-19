#include "mixer/auxiliary.h"

#include "control/controlproxy.h"
#include "engine/channels/engineaux.h"
#include "engine/enginemaster.h"
#include "moc_auxiliary.cpp"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"

Auxiliary::Auxiliary(QObject* pParent, const QString& group, int index,
                     SoundManager* pSoundManager, EngineMaster* pEngine,
                     EffectsManager* pEffectsManager)
        : BasePlayer(pParent, group) {
    ChannelHandleAndGroup channelGroup = pEngine->registerChannelGroup(group);
    EngineAux* pAuxiliary = new EngineAux(channelGroup, pEffectsManager);
    pEngine->addChannel(pAuxiliary);
    AudioInput auxInput = AudioInput(AudioPath::AUXILIARY, 0, 2, index);
    pSoundManager->registerInput(auxInput, pAuxiliary);

    m_pInputConfigured = make_parented<ControlProxy>(group, "input_configured", this);
    m_pAuxMasterEnabled = make_parented<ControlProxy>(group, "master", this);
    m_pAuxMasterEnabled->connectValueChanged(this, &Auxiliary::slotAuxMasterEnabled);
}

Auxiliary::~Auxiliary() {
}

void Auxiliary::slotAuxMasterEnabled(double v) {
    bool configured = m_pInputConfigured->toBool();
    bool auxMasterEnable = v > 0.0;

    // Warn the user if they try to enable master on a auxiliary with no
    // configured input.
    if (!configured && auxMasterEnable) {
        m_pAuxMasterEnabled->set(0.0);
        emit noAuxiliaryInputConfigured();
    }
}
