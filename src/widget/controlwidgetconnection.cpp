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

void ControlParameterWidgetConnection::setControlParameter(double v) {
    if (m_bConnectValueFromWidget) {
        m_pControl->setParameter(v);
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
                                                                 const QString& propertyName)
        : ControlWidgetConnection(pBaseWidget, pControl),
          m_propertyName(propertyName.toAscii()) {
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
}

void ControlWidgetPropertyConnection::resetControl() {
    // Do nothing.
}

void ControlWidgetPropertyConnection::setControlParameter(double v) {
    // Do nothing.
    Q_UNUSED(v);
}

void ControlWidgetPropertyConnection::setControlParameterDown(double v) {
    // Do nothing.
    Q_UNUSED(v);
}

void ControlWidgetPropertyConnection::setControlParameterUp(double v) {
    // Do nothing.
    Q_UNUSED(v);
}
