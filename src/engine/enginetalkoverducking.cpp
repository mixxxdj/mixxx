#include "engine/enginetalkoverducking.h"

#include "control/controllinpotmeter.h"
#include "control/controlproxy.h"
#include "moc_enginetalkoverducking.cpp"

namespace {

constexpr CSAMPLE kDuckThreshold = 0.1f;

} // namespace

EngineTalkoverDucking::EngineTalkoverDucking(
        UserSettingsPointer pConfig, const QString& group)
        : EngineSideChainCompressor(group),
          m_pConfig(pConfig),
          m_group(group) {
    m_pSampleRate = make_parented<ControlProxy>(
            QStringLiteral("[App]"), QStringLiteral("samplerate"), this);
    m_pSampleRate->connectValueChanged(this,
            &EngineTalkoverDucking::slotSampleRateChanged,
            Qt::DirectConnection);

    m_pDuckStrength = std::make_unique<ControlPotmeter>(
            ConfigKey(m_group, QStringLiteral("duckStrength")),
            0.0,
            1.0,
            false,
            true,
            false,
            true,
            0.9);
    connect(m_pDuckStrength.get(),
            &ControlObject::valueChanged,
            this,
            &EngineTalkoverDucking::slotDuckStrengthChanged,
            Qt::DirectConnection);

    // TODO Add "duckThreshold"

    m_pDuckAttackTime = std::make_unique<ControlPotmeter>(
            ConfigKey(m_group, QStringLiteral("duckAttackTime")),
            0.0,
            1.0,
            false,
            true,
            false,
            true,
            0.5);
    connect(m_pDuckAttackTime.get(),
            &ControlObject::valueChanged,
            this,
            &EngineTalkoverDucking::slotDuckAttackTimeChanged,
            Qt::DirectConnection);

    m_pDuckDecayTime = std::make_unique<ControlPotmeter>(
            ConfigKey(m_group, QStringLiteral("duckDecayTime")),
            0.0,
            1.0,
            false,
            true,
            false,
            true,
            0.5);
    connect(m_pDuckDecayTime.get(),
            &ControlObject::valueChanged,
            this,
            &EngineTalkoverDucking::slotDuckDecayTimeChanged,
            Qt::DirectConnection);

    slotSampleRateChanged(m_pSampleRate->get());

    m_pTalkoverDucking = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, QStringLiteral("talkoverDucking")), true, AUTO);
    m_pTalkoverDucking->setBehavior(mixxx::control::ButtonMode::Toggle, 3);
}

void EngineTalkoverDucking::slotSampleRateChanged(double samplerate) {
    qWarning() << ".";
    qWarning() << "EngineTalkoverDucking::slotSampleRateChanged:" << samplerate;
    qWarning() << ".";
    setParameters(
            kDuckThreshold,
            static_cast<CSAMPLE>(m_pDuckStrength->get()),
            static_cast<unsigned int>(m_pDuckAttackTime->get() * samplerate / 2 * .1),
            static_cast<unsigned int>(m_pDuckDecayTime->get() * samplerate / 2));
}

void EngineTalkoverDucking::slotDuckStrengthChanged(double strength) {
    setStrength(static_cast<CSAMPLE>(strength));
}

void EngineTalkoverDucking::slotDuckAttackTimeChanged(double attackTime) {
    setAttackTime(static_cast<unsigned int>(m_pSampleRate->get() / 2 * attackTime));
}

void EngineTalkoverDucking::slotDuckDecayTimeChanged(double decayTime) {
    setDecayTime(static_cast<unsigned int>(m_pSampleRate->get() / 2 * decayTime));
}

CSAMPLE EngineTalkoverDucking::getGain(int numFrames) {
    // Apply microphone ducking.
    switch (getMode()) {
    case EngineTalkoverDucking::OFF:
        return 1.0f;
    case EngineTalkoverDucking::AUTO:
    case EngineTalkoverDucking::MANUAL:
        return static_cast<CSAMPLE>(calculateCompressedGain(numFrames));
    default:
        DEBUG_ASSERT("!Unknown Ducking mode");
        return 1.0f;
    }
}
