#include "qml/qmlskincontrolcreator.h"

#include <QtDebug>

#include "control/controlobject.h"
#include "moc_qmlskincontrolcreator.cpp"
#include "util/assert.h"

namespace {
const QString kSkinGroup = QStringLiteral("[Skin]");
} // namespace

namespace mixxx {
namespace qml {

QmlSkinControlCreator::QmlSkinControlCreator(QObject* parent)
        : QObject(parent),
          m_persist(false),
          m_defaultValue(0.0),
          m_buttonMode(ButtonMode::Toggle),
          m_isComponentComplete(false) {
}

void QmlSkinControlCreator::componentComplete() {
    m_isComponentComplete = true;
    createControl();
}

void QmlSkinControlCreator::setGroup(const QString& group) {
    if (m_key.group == group) {
        return;
    }
    m_key.group = group;
    emit groupChanged(group);
    createControl();
}

const QString& QmlSkinControlCreator::getGroup() const {
    return m_key.group;
}

void QmlSkinControlCreator::setKey(const QString& key) {
    if (m_key.item == key) {
        return;
    }
    m_key.item = key;
    emit keyChanged(key);
    createControl();
}

const QString& QmlSkinControlCreator::getKey() const {
    return m_key.item;
}

void QmlSkinControlCreator::setPersist(bool persist) {
    if (m_persist == persist) {
        return;
    }
    m_persist = persist;
    emit persistChanged(persist);
    createControl();
}

bool QmlSkinControlCreator::getPersist() const {
    return m_persist;
}

void QmlSkinControlCreator::setDefaultValue(double defaultValue) {
    if (m_defaultValue == defaultValue) {
        return;
    }
    m_defaultValue = defaultValue;
    emit defaultValueChanged(defaultValue);
    createControl();
}

double QmlSkinControlCreator::getDefaultValue() const {
    return m_defaultValue;
}

void QmlSkinControlCreator::setButtonMode(ButtonMode buttonMode) {
    if (m_buttonMode == buttonMode) {
        return;
    }
    m_buttonMode = buttonMode;
    emit buttonModeChanged(buttonMode);
    if (m_pControl) {
        m_pControl->setButtonMode(toControlButtonMode(m_buttonMode));
    }
}

QmlSkinControlCreator::ButtonMode QmlSkinControlCreator::getButtonMode() const {
    return m_buttonMode;
}

mixxx::control::ButtonMode QmlSkinControlCreator::toControlButtonMode(
        ButtonMode buttonMode) {
    switch (buttonMode) {
    case ButtonMode::Push:
        return mixxx::control::ButtonMode::Push;
    case ButtonMode::Toggle:
        return mixxx::control::ButtonMode::Toggle;
    case ButtonMode::PowerWindow:
        return mixxx::control::ButtonMode::PowerWindow;
    case ButtonMode::LongPressLatching:
        return mixxx::control::ButtonMode::LongPressLatching;
    case ButtonMode::Trigger:
        return mixxx::control::ButtonMode::Trigger;
    }
    DEBUG_ASSERT(false);
    return mixxx::control::ButtonMode::Toggle;
}

void QmlSkinControlCreator::createControl() {
    if (!m_isComponentComplete) {
        return;
    }
    m_pControl.reset();

    if (!m_key.isValid()) {
        return;
    }
    if (m_key.group != kSkinGroup) {
        qWarning() << "QmlSkinControlCreator: Cannot create non-skin control"
                   << m_key.group << m_key.item;
        return;
    }
    if (ControlObject::exists(m_key)) {
        qWarning() << "QmlSkinControlCreator: Cannot create already existing skin control"
                   << m_key.group << m_key.item;
        return;
    }

    m_pControl = std::make_unique<ControlPushButton>(
            m_key,
            m_persist,
            m_defaultValue);
    m_pControl->setButtonMode(toControlButtonMode(m_buttonMode));
}

} // namespace qml
} // namespace mixxx
