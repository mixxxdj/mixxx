#include "engine/channels/enginechannel.h"

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "moc_enginechannel.cpp"

EngineChannel::EngineChannel(const ChannelHandleAndGroup& handleGroup,
        EngineChannel::ChannelOrientation defaultOrientation,
        EffectsManager* pEffectsManager,
        bool isTalkoverChannel,
        bool isPrimaryDeck)
        : m_group(handleGroup),
          m_pEffectsManager(pEffectsManager),
          m_vuMeter(getGroup()),
          m_pSampleRate(new ControlProxy("[Master]", "samplerate")),
          m_sampleBuffer(nullptr),
          m_bIsPrimaryDeck(isPrimaryDeck),
          m_bIsTalkoverChannel(isTalkoverChannel) {
    m_pPFL = new ControlPushButton(ConfigKey(getGroup(), "pfl"));
    m_pPFL->setButtonMode(ControlPushButton::TOGGLE);
    m_pMaster = new ControlPushButton(ConfigKey(getGroup(), "master"));
    m_pMaster->setButtonMode(ControlPushButton::POWERWINDOW);
    m_pOrientation = new ControlPushButton(ConfigKey(getGroup(), "orientation"));
    m_pOrientation->setButtonMode(ControlPushButton::TOGGLE);
    m_pOrientation->setStates(3);
    m_pOrientation->set(defaultOrientation);
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
    m_pTalkover->setButtonMode(ControlPushButton::POWERWINDOW);

    if (m_pEffectsManager != nullptr) {
        m_pEffectsManager->registerInputChannel(handleGroup);
    }
}

EngineChannel::~EngineChannel() {
    delete m_pMaster;
    delete m_pPFL;
    delete m_pOrientation;
    delete m_pOrientationLeft;
    delete m_pOrientationRight;
    delete m_pOrientationCenter;
    delete m_pSampleRate;
    delete m_pTalkover;
}

void EngineChannel::setPfl(bool enabled) {
    m_pPFL->set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isPflEnabled() const {
    return m_pPFL->toBool();
}

void EngineChannel::setMaster(bool enabled) {
    m_pMaster->set(enabled ? 1.0 : 0.0);
}

bool EngineChannel::isMasterEnabled() const {
    return m_pMaster->toBool();
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
    double dOrientation = m_pOrientation->get();
    if (dOrientation == LEFT) {
        return LEFT;
    } else if (dOrientation == CENTER) {
        return CENTER;
    } else if (dOrientation == RIGHT) {
        return RIGHT;
    }
    return CENTER;
}
