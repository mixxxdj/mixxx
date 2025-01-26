#include "engine/channels/enginechannel.h"

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "effects/effectsmanager.h"
#include "moc_enginechannel.cpp"

EngineChannel::EngineChannel(const ChannelHandleAndGroup& handleGroup,
        EngineChannel::ChannelOrientation defaultOrientation,
        EffectsManager* pEffectsManager,
        bool isTalkoverChannel,
        bool isPrimaryDeck)
        : m_group(handleGroup),
          m_pEffectsManager(pEffectsManager),
          m_vuMeter(getGroup()),
          m_sampleRate(QStringLiteral("[App]"), QStringLiteral("samplerate")),
          m_sampleBuffer(nullptr),
          m_bIsPrimaryDeck(isPrimaryDeck),
          m_active(false),
          m_pMainMix(std::make_unique<ControlPushButton>(
                  ConfigKey(getGroup(), QStringLiteral("main_mix")))),
          m_pPFL(std::make_unique<ControlPushButton>(
                  ConfigKey(getGroup(), QStringLiteral("pfl")))),
          // crossfader assignment is persistent
          m_pOrientation(std::make_unique<ControlPushButton>(
                  ConfigKey(getGroup(), QStringLiteral("orientation")),
                  true,
                  defaultOrientation)),
          m_pOrientationLeft(std::make_unique<ControlPushButton>(
                  ConfigKey(getGroup(), QStringLiteral("orientation_left")))),
          m_pOrientationRight(std::make_unique<ControlPushButton>(
                  ConfigKey(getGroup(), QStringLiteral("orientation_right")))),
          m_pOrientationCenter(std::make_unique<ControlPushButton>(
                  ConfigKey(getGroup(), QStringLiteral("orientation_center")))),
          m_pTalkover(std::make_unique<ControlPushButton>(
                  ConfigKey(getGroup(), QStringLiteral("talkover")))),
          m_bIsTalkoverChannel(isTalkoverChannel),
          m_channelIndex(-1) {
    m_pPFL->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pMainMix->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
    m_pMainMix->addAlias(ConfigKey(getGroup(), QStringLiteral("master")));
    m_pOrientation->setBehavior(mixxx::control::ButtonMode::Toggle, 3);
    connect(m_pOrientationLeft.get(),
            &ControlObject::valueChanged,
            this,
            &EngineChannel::slotOrientationLeft,
            Qt::DirectConnection);
    connect(m_pOrientationRight.get(),
            &ControlObject::valueChanged,
            this,
            &EngineChannel::slotOrientationRight,
            Qt::DirectConnection);
    connect(m_pOrientationCenter.get(),
            &ControlObject::valueChanged,
            this,
            &EngineChannel::slotOrientationCenter,
            Qt::DirectConnection);
    m_pTalkover->setButtonMode(mixxx::control::ButtonMode::PowerWindow);

    if (m_pEffectsManager != nullptr) {
        m_pEffectsManager->registerInputChannel(handleGroup);
    }
}

void EngineChannel::setPfl(bool enabled) {
    m_pPFL->set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isPflEnabled() const {
    return m_pPFL->toBool();
}

void EngineChannel::setMainMix(bool enabled) {
    m_pMainMix->set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isMainMixEnabled() const {
    return m_pMainMix->toBool();
}

void EngineChannel::setTalkover(bool enabled) {
    m_pTalkover->set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isTalkoverEnabled() const {
    return m_pTalkover->toBool();
}

void EngineChannel::slotOrientationLeft(double v) {
    if (v > 0) {
        m_pOrientation->set(LEFT);
    }
}

void EngineChannel::slotOrientationRight(double v) {
    if (v > 0) {
        m_pOrientation->set(RIGHT);
    }
}

void EngineChannel::slotOrientationCenter(double v) {
    if (v > 0) {
        m_pOrientation->set(CENTER);
    }
}

EngineChannel::ChannelOrientation EngineChannel::getOrientation() const {
    const double dOrientation = m_pOrientation->get();
    const int iOrientation = static_cast<int>(dOrientation);
    if (dOrientation != iOrientation) {
        return CENTER;
    }
    switch (iOrientation) {
    case LEFT:
        return LEFT;
    case CENTER:
        return CENTER;
    case RIGHT:
        return RIGHT;
    default:
        return CENTER;
    }
    return CENTER;
}
