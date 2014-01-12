#include "widget/controlwidgetconnection.h"

#include "widget/wbasewidget.h"
#include "controlobjectslave.h"
#include "util/debug.h"

ControlWidgetConnection::ControlWidgetConnection(WBaseWidget* pBaseWidget,
                                                 ControlObjectSlave* pControl)
        : m_pWidget(pBaseWidget),
          m_pControl(pControl) {
    // If pControl is NULL then the creator of ControlWidgetConnection has
    // screwed up badly enough that we should just crash. This will not go
    // unnoticed in development.
    Q_ASSERT(pControl);
    pControl->connectValueChanged(this, SLOT(slotControlValueChanged(double)));
}

ControlWidgetConnection::~ControlWidgetConnection() {
}

double ControlWidgetConnection::getControlParameter() const {
    return m_pControl->getParameter();
}

ControlParameterWidgetConnection::ControlParameterWidgetConnection(WBaseWidget* pBaseWidget,
                                                                   ControlObjectSlave* pControl,
                                                                   bool connectValueFromWidget,
                                                                   bool connectValueToWidget,
                                                                   EmitOption emitOption)
        : ControlWidgetConnection(pBaseWidget, pControl),
          m_bConnectValueFromWidget(connectValueFromWidget),
          m_bConnectValueToWidget(connectValueToWidget),
          m_emitOption(emitOption) {
    if (m_bConnectValueToWidget) {
        slotControlValueChanged(m_pControl->get());
    }
}

ControlParameterWidgetConnection::~ControlParameterWidgetConnection() {
}

QString ControlParameterWidgetConnection::toDebugString() const {
    const ConfigKey& key = m_pControl->getKey();
    return QString("%1,%2 Parameter: %3 ToWidget: %4 FromWidget %5 Emit: %6")
            .arg(key.group, key.item,
                 QString::number(m_pControl->getParameter()),
                 ::toDebugString(m_bConnectValueToWidget),
                 ::toDebugString(m_bConnectValueFromWidget),
                 emitOptionToString(m_emitOption));
}

void ControlParameterWidgetConnection::slotControlValueChanged(double v) {
    if (m_bConnectValueToWidget) {
        m_pWidget->onConnectedControlValueChanged(
                m_pControl->getParameterForValue(v));
        // TODO(rryan): copied from WWidget. Keep?
        //m_pWidget->toQWidget()->update();
    }
}

void ControlParameterWidgetConnection::resetControl() {
    if (m_bConnectValueFromWidget) {
        m_pControl->reset();
    }
}

void ControlParameterWidgetConnection::setControlParameterDown(double v) {
    if (m_bConnectValueFromWidget && m_emitOption & EMIT_ON_PRESS) {
        m_pControl->setParameter(v);
    }
}

void ControlParameterWidgetConnection::setControlParameterUp(double v) {
    if (m_bConnectValueFromWidget && m_emitOption & EMIT_ON_RELEASE) {
        m_pControl->setParameter(v);
    }
}

ControlWidgetPropertyConnection::ControlWidgetPropertyConnection(WBaseWidget* pBaseWidget,
                                                                 ControlObjectSlave* pControl,
                                                                 ConfigObject<ConfigValue>* pConfig,
                                                                 const QString& propertyName)
        : ControlWidgetConnection(pBaseWidget, pControl),
          m_pConfig(pConfig),
          m_propertyName(propertyName.toAscii()) {
    // Behavior copied from PropertyBinder: load config value for the control on
    // creation.
    // TODO(rryan): Remove this in favor of a better solution. See discussion on
    // Bug #1091147.
    bool ok = false;
    double dValue = m_pConfig->getValueString(m_pControl->getKey()).toDouble(&ok);
    if (ok) {
        m_pControl->setParameter(dValue);
    }
    slotControlValueChanged(m_pControl->get());
}

ControlWidgetPropertyConnection::~ControlWidgetPropertyConnection() {
}

QString ControlWidgetPropertyConnection::toDebugString() const {
    const ConfigKey& key = m_pControl->getKey();
    return QString("%1,%2 Parameter: %3 Property: %4 Value: %5").arg(
        key.group, key.item, QString::number(m_pControl->getParameter()), m_propertyName,
        m_pWidget->toQWidget()->property(
            m_propertyName.constData()).toString());
}

void ControlWidgetPropertyConnection::slotControlValueChanged(double v) {
    double dParameter = m_pControl->getParameterForValue(v);

    if (!m_pWidget->toQWidget()->setProperty(m_propertyName.constData(),
                                             QVariant(dParameter))) {
        qDebug() << "Setting property" << m_propertyName
                 << "to widget failed. Value:" << dParameter;
    }

    // Behavior copied from PropertyBinder: save config value for the control on
    // every change.
    // TODO(rryan): Remove this in favor of a better solution. See discussion on
    // Bug #1091147.
    m_pConfig->set(m_pControl->getKey(), QString::number(dParameter));
}

void ControlWidgetPropertyConnection::resetControl() {
    // Do nothing.
}

void ControlWidgetPropertyConnection::setControlParameterDown(double v) {
    // Do nothing.
    Q_UNUSED(v);
}

void ControlWidgetPropertyConnection::setControlParameterUp(double v) {
    // Do nothing.
    Q_UNUSED(v);
}
