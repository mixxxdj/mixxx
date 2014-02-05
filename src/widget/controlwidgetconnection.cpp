#include "widget/controlwidgetconnection.h"

#include "widget/wbasewidget.h"
#include "controlobjectslave.h"
#include "util/debug.h"
#include "util/valuetransformer.h"

ControlWidgetConnection::ControlWidgetConnection(WBaseWidget* pBaseWidget,
                                                 ControlObjectSlave* pControl,
                                                 ValueTransformer* pTransformer)
        : m_pWidget(pBaseWidget),
          m_pControl(pControl),
          m_pValueTransformer(pTransformer) {
    // If pControl is NULL then the creator of ControlWidgetConnection has
    // screwed up badly enough that we should just crash. This will not go
    // unnoticed in development.
    Q_ASSERT(pControl);
    pControl->connectValueChanged(this, SLOT(slotControlValueChanged(double)));
}

ControlWidgetConnection::~ControlWidgetConnection() {
}

void ControlWidgetConnection::setControlParameter(double parameter) {
    if (m_pValueTransformer != NULL) {
        parameter = m_pValueTransformer->transformInverse(parameter);
    }
    m_pControl->setParameter(parameter);
}

double ControlWidgetConnection::getControlParameter() const {
    double parameter = m_pControl->getParameter();
    if (m_pValueTransformer != NULL) {
        parameter = m_pValueTransformer->transform(parameter);
    }
    return parameter;
}

double ControlWidgetConnection::getControlParameterForValue(double value) const {
    double parameter = m_pControl->getParameterForValue(value);
    if (m_pValueTransformer != NULL) {
        parameter = m_pValueTransformer->transform(parameter);
    }
    return parameter;
}

ControlParameterWidgetConnection::ControlParameterWidgetConnection(WBaseWidget* pBaseWidget,
                                                                   ControlObjectSlave* pControl,
                                                                   ValueTransformer* pTransformer,
                                                                   bool connectValueFromWidget,
                                                                   bool connectValueToWidget,
                                                                   EmitOption emitOption)
        : ControlWidgetConnection(pBaseWidget, pControl, pTransformer),
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
        double parameter = getControlParameterForValue(v);
        m_pWidget->onConnectedControlValueChanged(parameter);
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
        setControlParameter(v);
    }
}

void ControlParameterWidgetConnection::setControlParameterUp(double v) {
    if (m_bConnectValueFromWidget && m_emitOption & EMIT_ON_RELEASE) {
        setControlParameter(v);
    }
}

ControlWidgetPropertyConnection::ControlWidgetPropertyConnection(WBaseWidget* pBaseWidget,
                                                                 ControlObjectSlave* pControl,
                                                                 ValueTransformer* pTransformer,
                                                                 const QString& propertyName)
        : ControlWidgetConnection(pBaseWidget, pControl, pTransformer),
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
    double dParameter = getControlParameterForValue(v);

    if (!m_pWidget->toQWidget()->setProperty(m_propertyName.constData(),
                                             QVariant(dParameter))) {
        qDebug() << "Setting property" << m_propertyName
                 << "to widget failed. Value:" << dParameter;
    }
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
