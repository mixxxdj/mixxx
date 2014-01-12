#include <QtDebug>

#include "widget/wbasewidget.h"

#include "widget/controlwidgetconnection.h"
#include "util/cmdlineargs.h"
#include "util/debug.h"

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
