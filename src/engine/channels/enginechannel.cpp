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
          m_sampleRate({QStringLiteral("[App]"), QStringLiteral("samplerate")}),
          m_sampleBuffer(nullptr),
          m_bIsPrimaryDeck(isPrimaryDeck),
          m_active(false),
          m_bIsTalkoverChannel(isTalkoverChannel),
          m_channelIndex(-1) {
    m_pPFL = new ControlPushButton(ConfigKey(getGroup(), "pfl"));
    m_pPFL->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pMainMix = new ControlPushButton(ConfigKey(getGroup(), "main_mix"));
    m_pMainMix->setButtonMode(mixxx::control::ButtonMode::PowerWindow);
    m_pMainMix->addAlias(ConfigKey(getGroup(), QStringLiteral("master")));
    // crossfader assignment is persistent
    m_pOrientation = new ControlPushButton(
            ConfigKey(getGroup(), "orientation"), true, defaultOrientation);
    m_pOrientation->setBehavior(mixxx::control::ButtonMode::Toggle, 3);
    m_pOrientationLeft = new ControlPushButton(ConfigKey(getGroup(), "orientation_left"));
    connect(m_pOrientationLeft, &ControlObject::valueChanged,
            this, &EngineChannel::slotOrientationLeft, Qt::DirectConnection);
    m_pOrientationRight = new ControlPushButton(ConfigKey(getGroup(), "orientation_right"));
    connect(m_pOrientationRight, &ControlObject::valueChanged,
            this, &EngineChannel::slotOrientationRight, Qt::DirectConnection);
    m_pOrientationCenter = new ControlPushButton(ConfigKey(getGroup(), "orientation_center"));
    connect(m_pOrientationCenter, &ControlObject::valueChanged,
            this, &EngineChannel::slotOrientationCenter, Qt::DirectConnection);
    m_pTalkover = new ControlPushButton(ConfigKey(getGroup(), "talkover"));
    m_pTalkover->setButtonMode(mixxx::control::ButtonMode::PowerWindow);

    if (m_pEffectsManager != nullptr) {
        m_pEffectsManager->registerInputChannel(handleGroup);
    }
}

EngineChannel::~EngineChannel() {
    delete m_pMainMix;
    delete m_pPFL;
    delete m_pOrientation;
    delete m_pOrientationLeft;
    delete m_pOrientationRight;
    delete m_pOrientationCenter;
    delete m_pTalkover;
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
