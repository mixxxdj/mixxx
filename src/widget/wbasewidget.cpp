#include <QtDebug>

#include "widget/wbasewidget.h"

#include "controlobjectslave.h"

ControlWidgetConnection::ControlWidgetConnection(WBaseWidget* pBaseWidget,
                                                 ControlObjectSlave* pControl)
        : m_pWidget(pBaseWidget),
          m_pControl(pControl) {
    pControl->connectValueChanged(this, SLOT(slotControlValueChanged(double)));
}

ControlWidgetConnection::~ControlWidgetConnection() {
}

double ControlWidgetConnection::get() const {
    return m_pControl->get();
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

void ValueControlWidgetConnection::setDown(double v) {
    if (m_bConnectValueFromWidget && m_emitOption & EMIT_ON_PRESS) {
        m_pControl->setParameter(v);
    }
}

void ValueControlWidgetConnection::setUp(double v) {
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

void DisabledControlWidgetConnection::setDown(double v) {
    // Do nothing.
    Q_UNUSED(v);
}

void DisabledControlWidgetConnection::setUp(double v) {
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

double WBaseWidget::getConnectedControl() const {
    if (!m_leftConnections.isEmpty()) {
        return m_leftConnections[0]->get();
    }
    if (!m_rightConnections.isEmpty()) {
        return m_rightConnections[0]->get();
    }
    if (!m_connections.isEmpty()) {
        return m_connections[0]->get();
    }
    return 0.0;
}

double WBaseWidget::getConnectedControlLeft() const {
    if (!m_leftConnections.isEmpty()) {
        return m_leftConnections[0]->get();
    }
    if (!m_connections.isEmpty()) {
        return m_connections[0]->get();
    }
    return 0.0;
}

double WBaseWidget::getConnectedControlRight() const {
    if (!m_rightConnections.isEmpty()) {
        return m_rightConnections[0]->get();
    }
    if (!m_connections.isEmpty()) {
        return m_connections[0]->get();
    }
    return 0.0;
}

double WBaseWidget::getConnectedDisplayValue() const {
    if (m_pDisplayConnection) {
        return m_pDisplayConnection->get();
    }
    return 0.0;
}

void WBaseWidget::resetConnectedControls() {
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

void WBaseWidget::setConnectedControlDown(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->setDown(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->setDown(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setDown(v);
    }
}

void WBaseWidget::setConnectedControlUp(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->setUp(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->setUp(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setUp(v);
    }
}

void WBaseWidget::setConnectedControlLeftDown(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->setDown(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setDown(v);
    }
}

void WBaseWidget::setConnectedControlLeftUp(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->setUp(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setUp(v);
    }
}

void WBaseWidget::setConnectedControlRightDown(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->setDown(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setDown(v);
    }
}

void WBaseWidget::setConnectedControlRightUp(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->setUp(v);
    }
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setUp(v);
    }
}
