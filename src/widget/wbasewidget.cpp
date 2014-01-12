#include <QtDebug>

#include "widget/wbasewidget.h"

#include "controlobjectslave.h"
#include "util/cmdlineargs.h"
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

ValueControlWidgetConnection::ValueControlWidgetConnection(WBaseWidget* pBaseWidget,
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

ValueControlWidgetConnection::~ValueControlWidgetConnection() {
}

QString ValueControlWidgetConnection::toDebugString() const {
    const ConfigKey& key = m_pControl->getKey();
    return QString("%1,%2 Parameter: %3 ToWidget: %4 FromWidget %5 Emit: %6")
            .arg(key.group, key.item,
                 QString::number(m_pControl->getParameter()),
                 ::toDebugString(m_bConnectValueToWidget),
                 ::toDebugString(m_bConnectValueFromWidget),
                 emitOptionToString(m_emitOption));
}

void ValueControlWidgetConnection::slotControlValueChanged(double v) {
    if (m_bConnectValueToWidget) {
        m_pWidget->onConnectedControlValueChanged(
                m_pControl->getParameterForValue(v));
        // TODO(rryan): copied from WWidget. Keep?
        //m_pWidget->toQWidget()->update();
    }
}

void ValueControlWidgetConnection::resetControl() {
    if (m_bConnectValueFromWidget) {
        m_pControl->reset();
    }
}

void ValueControlWidgetConnection::setControlParameterDown(double v) {
    if (m_bConnectValueFromWidget && m_emitOption & EMIT_ON_PRESS) {
        m_pControl->setParameter(v);
    }
}

void ValueControlWidgetConnection::setControlParameterUp(double v) {
    if (m_bConnectValueFromWidget && m_emitOption & EMIT_ON_RELEASE) {
        m_pControl->setParameter(v);
    }
}

PropertyControlWidgetConnection::PropertyControlWidgetConnection(WBaseWidget* pBaseWidget,
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

PropertyControlWidgetConnection::~PropertyControlWidgetConnection() {
}

QString PropertyControlWidgetConnection::toDebugString() const {
    const ConfigKey& key = m_pControl->getKey();
    return QString("%1,%2 Parameter: %3 Property: %4 Value: %5").arg(
        key.group, key.item, QString::number(m_pControl->getParameter()), m_propertyName,
        m_pWidget->toQWidget()->property(
            m_propertyName.constData()).toString());
}

void PropertyControlWidgetConnection::slotControlValueChanged(double v) {
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

void PropertyControlWidgetConnection::resetControl() {
    // Do nothing.
}

void PropertyControlWidgetConnection::setControlParameterDown(double v) {
    // Do nothing.
    Q_UNUSED(v);
}

void PropertyControlWidgetConnection::setControlParameterUp(double v) {
    // Do nothing.
    Q_UNUSED(v);
}

WBaseWidget::WBaseWidget(QWidget* pWidget)
        : m_pWidget(pWidget),
          m_pDisplayConnection(NULL) {
}

WBaseWidget::~WBaseWidget() {
    m_pDisplayConnection = NULL;
    while (!m_leftConnections.isEmpty()) {
        delete m_leftConnections.takeLast();
    }
    while (!m_rightConnections.isEmpty()) {
        delete m_rightConnections.takeLast();
    }
    while (!m_connections.isEmpty()) {
        delete m_connections.takeLast();
    }
}

void WBaseWidget::setDisplayConnection(ControlWidgetConnection* pConnection) {
    m_pDisplayConnection = pConnection;
}

void WBaseWidget::addConnection(ControlWidgetConnection* pConnection) {
    m_connections.append(pConnection);
}

void WBaseWidget::addLeftConnection(ControlWidgetConnection* pConnection) {
    m_leftConnections.append(pConnection);
}

void WBaseWidget::addRightConnection(ControlWidgetConnection* pConnection) {
    m_rightConnections.append(pConnection);
}

double WBaseWidget::getControlParameter() const {
    if (!m_leftConnections.isEmpty()) {
        return m_leftConnections.at(0)->getControlParameter();
    }
    if (!m_rightConnections.isEmpty()) {
        return m_rightConnections.at(0)->getControlParameter();
    }
    if (!m_connections.isEmpty()) {
        return m_connections.at(0)->getControlParameter();
    }
    return 0.0;
}

double WBaseWidget::getControlParameterLeft() const {
    if (!m_leftConnections.isEmpty()) {
        return m_leftConnections.at(0)->getControlParameter();
    }
    if (!m_connections.isEmpty()) {
        return m_connections.at(0)->getControlParameter();
    }
    return 0.0;
}

double WBaseWidget::getControlParameterRight() const {
    if (!m_rightConnections.isEmpty()) {
        return m_rightConnections.at(0)->getControlParameter();
    }
    if (!m_connections.isEmpty()) {
        return m_connections.at(0)->getControlParameter();
    }
    return 0.0;
}

double WBaseWidget::getControlParameterDisplay() const {
    if (m_pDisplayConnection != NULL) {
        return m_pDisplayConnection->getControlParameter();
    }
    return 0.0;
}

void WBaseWidget::resetControlParameters() {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->resetControl();
    }
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->resetControl();
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->resetControl();
    }
}

void WBaseWidget::setControlParameterDown(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->setControlParameterDown(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->setControlParameterDown(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setControlParameterDown(v);
    }
}

void WBaseWidget::setControlParameterUp(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->setControlParameterUp(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->setControlParameterUp(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setControlParameterUp(v);
    }
}

void WBaseWidget::setControlParameterLeftDown(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->setControlParameterDown(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setControlParameterDown(v);
    }
}

void WBaseWidget::setControlParameterLeftUp(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->setControlParameterUp(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setControlParameterUp(v);
    }
}

void WBaseWidget::setControlParameterRightDown(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->setControlParameterDown(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setControlParameterDown(v);
    }
}

void WBaseWidget::setControlParameterRightUp(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->setControlParameterUp(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setControlParameterUp(v);
    }
}

void WBaseWidget::updateTooltip() {
    // If we are in developer mode, update the tooltip.
    if (CmdlineArgs::Instance().getDeveloper()) {
        QStringList debug;
        fillDebugTooltip(&debug);

        QString base = baseTooltip();
        if (!base.isEmpty()) {
            debug.append(QString("Tooltip: \"%1\"").arg(base));
        }
        m_pWidget->setToolTip(debug.join("\n"));
    }
}

template <>
QString toDebugString(const QSizePolicy::Policy& policy) {
    switch (policy) {
        case QSizePolicy::Fixed:
            return "Fixed";
        case QSizePolicy::Minimum:
            return "Minimum";
        case QSizePolicy::Maximum:
            return "Maximum";
        case QSizePolicy::Preferred:
            return "Preferred";
        case QSizePolicy::Expanding:
            return "Expanding";
        case QSizePolicy::MinimumExpanding:
            return "MinimumExpanding";
        case QSizePolicy::Ignored:
            return "Ignored";
        default:
            break;
    }
    return QString::number(static_cast<int>(policy));
}

void WBaseWidget::fillDebugTooltip(QStringList* debug) {
    QSizePolicy policy = m_pWidget->sizePolicy();
    *debug << QString("ClassName: %1").arg(m_pWidget->metaObject()->className())
           << QString("ObjectName: %1").arg(m_pWidget->objectName())
           << QString("Position: %1").arg(toDebugString(m_pWidget->pos()))
           << QString("SizePolicy: %1,%2").arg(toDebugString(policy.horizontalPolicy()),
                                               toDebugString(policy.verticalPolicy()))
           << QString("Size: %1").arg(toDebugString(m_pWidget->size()))
           << QString("SizeHint: %1").arg(toDebugString(m_pWidget->sizeHint()))
           << QString("MinimumSizeHint: %1").arg(toDebugString(m_pWidget->minimumSizeHint()));

    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        *debug << QString("LeftConnection: %1").arg(pControlConnection->toDebugString());
    }
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        *debug << QString("RightConnection: %1").arg(pControlConnection->toDebugString());
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        *debug << QString("Connection: %1").arg(pControlConnection->toDebugString());
    }
    if (m_pDisplayConnection) {
        *debug << QString("DisplayConnection: %1").arg(m_pDisplayConnection->toDebugString());
    }
}
