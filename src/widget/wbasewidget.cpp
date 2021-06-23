#include <QtDebug>

#include "widget/wbasewidget.h"

#include "widget/controlwidgetconnection.h"
#include "util/cmdlineargs.h"
#include "util/debug.h"

WBaseWidget::WBaseWidget(QWidget* pWidget)
        : m_pDisplayConnection(nullptr),
          m_pWidget(pWidget) {
}

WBaseWidget::~WBaseWidget() {
    m_pDisplayConnection = nullptr;
    while (!m_leftConnections.isEmpty()) {
        delete m_leftConnections.takeLast();
    }
    while (!m_rightConnections.isEmpty()) {
        delete m_rightConnections.takeLast();
    }
    while (!m_connections.isEmpty()) {
        delete m_connections.takeLast();
    }
    while (!m_propertyConnections.isEmpty()) {
        delete m_propertyConnections.takeLast();
    }
}

void WBaseWidget::Init() {
    if (m_pDisplayConnection) {
        m_pDisplayConnection->Init();
    }
}

void WBaseWidget::setDisplayConnection(ControlParameterWidgetConnection* pConnection) {
    //qDebug() << "WBaseWidget::setDisplayConnection()" << pConnection->toDebugString();
    m_pDisplayConnection = pConnection;
}

void WBaseWidget::addConnection(ControlParameterWidgetConnection* pConnection) {
    m_connections.append(pConnection);
}

void WBaseWidget::addLeftConnection(ControlParameterWidgetConnection* pConnection) {
    m_leftConnections.append(pConnection);
}

void WBaseWidget::addRightConnection(ControlParameterWidgetConnection* pConnection) {
    m_rightConnections.append(pConnection);
}

void WBaseWidget::addPropertyConnection(ControlWidgetPropertyConnection* pConnection) {
    m_propertyConnections.append(pConnection);
}

double WBaseWidget::getControlParameter() const {
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
    return 0.0;
}

double WBaseWidget::getControlParameterDisplay() const {
    if (m_pDisplayConnection != nullptr) {
        return m_pDisplayConnection->getControlParameter();
    }
    return 0.0;
}

void WBaseWidget::resetControlParameter() {
    for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_connections)) {
        pControlConnection->resetControl();
    }
}

void WBaseWidget::setControlParameter(double v) {
    for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_connections)) {
        pControlConnection->setControlParameter(v);
    }
}

void WBaseWidget::setControlParameterUp(double v) {
    for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_connections)) {
        pControlConnection->setControlParameterUp(v);
    }
}

void WBaseWidget::setControlParameterDown(double v) {
    for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_connections)) {
        pControlConnection->setControlParameterDown(v);
    }
}

void WBaseWidget::setControlParameterLeftDown(double v) {
    if (!m_leftConnections.isEmpty()) {
        for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_leftConnections)) {
            pControlConnection->setControlParameterDown(v);
        }
    } else {
        for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_connections)) {
            pControlConnection->setControlParameterDown(v);
        }
    }
}

void WBaseWidget::setControlParameterLeftUp(double v) {
    if (!m_leftConnections.isEmpty()) {
        for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_leftConnections)) {
            pControlConnection->setControlParameterUp(v);
        }
    } else {
        for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_connections)) {
            pControlConnection->setControlParameterUp(v);
        }
    }
}

void WBaseWidget::setControlParameterRightDown(double v) {
    for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_rightConnections)) {
        pControlConnection->setControlParameterDown(v);
    }
}

void WBaseWidget::setControlParameterRightUp(double v) {
    for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_rightConnections)) {
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

    for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_leftConnections)) {
        *debug << QString("LeftConnection: %1").arg(pControlConnection->toDebugString());
    }
    for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_rightConnections)) {
        *debug << QString("RightConnection: %1").arg(pControlConnection->toDebugString());
    }
    for (ControlParameterWidgetConnection* pControlConnection : qAsConst(m_connections)) {
        *debug << QString("Connection: %1").arg(pControlConnection->toDebugString());
    }
    if (m_pDisplayConnection) {
        *debug << QString("DisplayConnection: %1").arg(m_pDisplayConnection->toDebugString());
    }
}
