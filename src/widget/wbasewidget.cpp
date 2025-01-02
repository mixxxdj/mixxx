#include "widget/wbasewidget.h"

#include "widget/controlwidgetconnection.h"
#include "util/cmdlineargs.h"
#include "util/debug.h"

WBaseWidget::WBaseWidget(QWidget* pWidget)
        : m_pDisplayConnection(nullptr),
          m_pWidget(pWidget) {
}

WBaseWidget::~WBaseWidget() = default;

void WBaseWidget::Init() {
    if (m_pDisplayConnection) {
        m_pDisplayConnection->Init();
    }
}

void WBaseWidget::addConnection(
        std::unique_ptr<ControlParameterWidgetConnection> pConnection,
        ConnectionSide side) {
    switch (side) {
    case ConnectionSide::None:
        m_connections.push_back(std::move(pConnection));
        break;
    case ConnectionSide::Left:
        m_leftConnections.push_back(std::move(pConnection));
        break;
    case ConnectionSide::Right:
        m_rightConnections.push_back(std::move(pConnection));
        break;
    }
}

void WBaseWidget::addAndSetDisplayConnection(
        std::unique_ptr<ControlParameterWidgetConnection> pConnection,
        ConnectionSide side) {
    m_pDisplayConnection = pConnection.get();
    addConnection(std::move(pConnection), side);
}

void WBaseWidget::addPropertyConnection(
        std::unique_ptr<ControlWidgetPropertyConnection> pConnection) {
    m_propertyConnections.push_back(std::move(pConnection));
}

double WBaseWidget::getControlParameter() const {
    if (!m_connections.empty()) {
        return m_connections.at(0)->getControlParameter();
    }
    return 0.0;
}

double WBaseWidget::getControlParameterLeft() const {
    if (!m_leftConnections.empty()) {
        return m_leftConnections.at(0)->getControlParameter();
    }
    if (!m_connections.empty()) {
        return m_connections.at(0)->getControlParameter();
    }
    return 0.0;
}

double WBaseWidget::getControlParameterRight() const {
    if (!m_rightConnections.empty()) {
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
    for (const auto& pControlConnection : std::as_const(m_connections)) {
        pControlConnection->resetControl();
    }
}

void WBaseWidget::setControlParameter(double v) {
    for (const auto& pControlConnection : std::as_const(m_connections)) {
        pControlConnection->setControlParameter(v);
    }
}

void WBaseWidget::setControlParameterUp(double v) {
    for (const auto& pControlConnection : std::as_const(m_connections)) {
        pControlConnection->setControlParameterUp(v);
    }
}

void WBaseWidget::setControlParameterDown(double v) {
    for (const auto& pControlConnection : std::as_const(m_connections)) {
        pControlConnection->setControlParameterDown(v);
    }
}

void WBaseWidget::setControlParameterLeftDown(double v) {
    if (!m_leftConnections.empty()) {
        for (const auto& pControlConnection :
                std::as_const(m_leftConnections)) {
            pControlConnection->setControlParameterDown(v);
        }
    } else {
        for (const auto& pControlConnection : std::as_const(m_connections)) {
            pControlConnection->setControlParameterDown(v);
        }
    }
}

void WBaseWidget::setControlParameterLeftUp(double v) {
    if (!m_leftConnections.empty()) {
        for (const auto& pControlConnection :
                std::as_const(m_leftConnections)) {
            pControlConnection->setControlParameterUp(v);
        }
    } else {
        for (const auto& pControlConnection : std::as_const(m_connections)) {
            pControlConnection->setControlParameterUp(v);
        }
    }
}

void WBaseWidget::setControlParameterRightDown(double v) {
    for (const auto& pControlConnection : std::as_const(m_rightConnections)) {
        pControlConnection->setControlParameterDown(v);
    }
}

void WBaseWidget::setControlParameterRightUp(double v) {
    for (const auto& pControlConnection : std::as_const(m_rightConnections)) {
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

    for (const auto& pControlConnection : std::as_const(m_leftConnections)) {
        *debug << QString("LeftConnection: %1").arg(pControlConnection->toDebugString());
    }
    for (const auto& pControlConnection : std::as_const(m_rightConnections)) {
        *debug << QString("RightConnection: %1").arg(pControlConnection->toDebugString());
    }
    for (const auto& pControlConnection : std::as_const(m_connections)) {
        *debug << QString("Connection: %1").arg(pControlConnection->toDebugString());
    }
    if (m_pDisplayConnection) {
        *debug << QString("DisplayConnection: %1").arg(m_pDisplayConnection->toDebugString());
    }
}
