#include <QtDebug>

#include "widget/wbasewidget.h"

#include "controlobjectslave.h"

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

void ValueControlWidgetConnection::slotControlValueChanged(double v) {
    if (m_bConnectValueToWidget) {
        m_pWidget->onConnectedControlValueChanged(
                m_pControl->getParameterForValue(v));
        // TODO(rryan): copied from WWidget. Keep?
        //m_pWidget->toQWidget()->update();
    }
}

void ValueControlWidgetConnection::reset() {
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

DisabledControlWidgetConnection::DisabledControlWidgetConnection(WBaseWidget* pBaseWidget,
                                                                 ControlObjectSlave* pControl)
        : ControlWidgetConnection(pBaseWidget, pControl) {
    slotControlValueChanged(m_pControl->get());
}

DisabledControlWidgetConnection::~DisabledControlWidgetConnection() {
}

void DisabledControlWidgetConnection::slotControlValueChanged(double v) {
    m_pWidget->setControlDisabled(m_pControl->getParameterForValue(v) != 0.0);
    m_pWidget->toQWidget()->update();
}

void DisabledControlWidgetConnection::reset() {
    // Do nothing.
}

void DisabledControlWidgetConnection::setControlParameterDown(double v) {
    // Do nothing.
    Q_UNUSED(v);
}

void DisabledControlWidgetConnection::setControlParameterUp(double v) {
    // Do nothing.
    Q_UNUSED(v);
}

WBaseWidget::WBaseWidget(QWidget* pWidget)
        : m_pWidget(pWidget),
          m_bDisabled(false),
          m_pDisplayConnection(NULL) {
}

WBaseWidget::~WBaseWidget() {
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
        pControlConnection->reset();
    }
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->reset();
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->reset();
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
