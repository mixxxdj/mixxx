#include "widget/controlwidgetconnection.h"

#include "widget/wbasewidget.h"
#include "controlobjectslave.h"
#include "util/debug.h"
#include "util/valuetransformer.h"
#include "util/assert.h"

ControlWidgetConnection::ControlWidgetConnection(WBaseWidget* pBaseWidget,
                                                 ControlObjectSlave* pControl,
                                                 ValueTransformer* pTransformer)
        : m_pWidget(pBaseWidget),
          m_pControl(pControl),
          m_pValueTransformer(pTransformer) {
    // If m_pControl is NULL then the creator of ControlWidgetConnection has
    // screwed up badly. Assert in development mode. In release mode the
    // connection will be defunct.
    DEBUG_ASSERT_AND_HANDLE(!m_pControl.isNull()) {
        m_pControl.reset(new ControlObjectSlave());
    }
    m_pControl->connectValueChanged(this, SLOT(slotControlValueChanged(double)));
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
                                                                   DirectionOption directionOption,
                                                                   EmitOption emitOption)
        : ControlWidgetConnection(pBaseWidget, pControl, pTransformer),
          m_directionOption(directionOption),
          m_emitOption(emitOption) {
}

ControlParameterWidgetConnection::~ControlParameterWidgetConnection() {
}

void ControlParameterWidgetConnection::Init() {
    slotControlValueChanged(m_pControl->get());
}

QString ControlParameterWidgetConnection::toDebugString() const {
    const ConfigKey& key = getKey();
    return QString("%1,%2 Parameter: %3 Direction: %4 Emit: %5")
            .arg(key.group, key.item,
                 QString::number(m_pControl->getParameter()),
                 directionOptionToString(m_directionOption),
                 emitOptionToString(m_emitOption));
}

void ControlParameterWidgetConnection::slotControlValueChanged(double value) {
    if (m_directionOption & DIR_TO_WIDGET) {
        double parameter = getControlParameterForValue(value);
        m_pWidget->onConnectedControlChanged(parameter, value);
    }
}

void ControlParameterWidgetConnection::resetControl() {
    if (m_directionOption & DIR_FROM_WIDGET) {
        m_pControl->reset();
    }
}

void ControlParameterWidgetConnection::setControlParameter(double v) {
    if (m_directionOption & DIR_FROM_WIDGET) {
        ControlWidgetConnection::setControlParameter(v);
    }
}

void ControlParameterWidgetConnection::setControlParameterDown(double v) {
    if ((m_directionOption & DIR_FROM_WIDGET) && (m_emitOption & EMIT_ON_PRESS)) {
        ControlWidgetConnection::setControlParameter(v);
    }
}

void ControlParameterWidgetConnection::setControlParameterUp(double v) {
    if ((m_directionOption & DIR_FROM_WIDGET) && (m_emitOption & EMIT_ON_RELEASE)) {
        ControlWidgetConnection::setControlParameter(v);
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
    const ConfigKey& key = getKey();
    return QString("%1,%2 Parameter: %3 Property: %4 Value: %5").arg(
        key.group, key.item, QString::number(m_pControl->getParameter()), m_propertyName,
        m_pWidget->toQWidget()->property(
            m_propertyName.constData()).toString());
}

void ControlWidgetPropertyConnection::slotControlValueChanged(double v) {
    QVariant parameter;
    QWidget* pWidget = m_pWidget->toQWidget();
    QVariant property = pWidget->property(m_propertyName.constData());
    if (property.type() == QVariant::Bool) {
        parameter = getControlParameterForValue(v) > 0;
    } else {
        parameter = getControlParameterForValue(v);
    }

    if (!pWidget->setProperty(m_propertyName.constData(),parameter)) {
        qDebug() << "Setting property" << m_propertyName
                << "to widget failed. Value:" << parameter;
    }
}
