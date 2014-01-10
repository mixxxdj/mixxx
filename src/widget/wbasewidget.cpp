#include <QtDebug>

#include "widget/wbasewidget.h"

#include "controlobjectslave.h"

ControlWidgetConnection::ControlWidgetConnection(WBaseWidget* pBaseWidget,
                                                 ControlObjectSlave* pControl)
        : m_pWidget(pBaseWidget),
          m_pControl(pControl) {
    pControl->connectParameterChanged(this, SLOT(slotControlValueChanged(double)));
}

ControlWidgetConnection::~ControlWidgetConnection() {
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
        slotControlValueChanged(m_pControl->getParameter());
    }
}

ValueControlWidgetConnection::~ValueControlWidgetConnection() {
}

void ValueControlWidgetConnection::slotControlValueChanged(double v) {
    if (m_bConnectValueToWidget) {
        m_pWidget->onConnectedControlValueChanged(v);
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
    slotControlValueChanged(m_pControl->getParameter());
}

DisabledControlWidgetConnection::~DisabledControlWidgetConnection() {
}

void DisabledControlWidgetConnection::slotControlValueChanged(double v) {
    m_pWidget->setControlDisabled(v != 0.0);
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
          m_bDisabled(false) {
}

WBaseWidget::~WBaseWidget() {
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
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setDown(v);
    }
}

void WBaseWidget::setConnectedControlUp(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_connections) {
        pControlConnection->setUp(v);
    }
}

void WBaseWidget::setConnectedControlLeftDown(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->setDown(v);
    }
    setConnectedControlDown(v);
}

void WBaseWidget::setConnectedControlLeftUp(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_leftConnections) {
        pControlConnection->setUp(v);
    }
    setConnectedControlUp(v);
}

void WBaseWidget::setConnectedControlRightDown(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->setDown(v);
    }
    setConnectedControlDown(v);
}

void WBaseWidget::setConnectedControlRightUp(double v) {
    foreach (ControlWidgetConnection* pControlConnection, m_rightConnections) {
        pControlConnection->setUp(v);
    }
    setConnectedControlUp(v);
}
