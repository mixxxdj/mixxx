#include "qml/qmlspinnyposition.h"

#include <QQuickWindow>
#include <QSGNode>
#include <cmath>

#include "moc_qmlspinnyposition.cpp"
#include "waveform/visualplayposition.h"

namespace mixxx {
namespace qml {

QmlSpinnyPosition::QmlSpinnyPosition(QQuickItem* parent)
        : QQuickItem(parent) {
    setFlag(QQuickItem::ItemHasContents, true);

    connect(this,
            &QQuickItem::windowChanged,
            this,
            &QmlSpinnyPosition::slotWindowChanged,
            Qt::DirectConnection);
    connect(this, &QQuickItem::visibleChanged, this, [this]() {
        if (isVisible()) {
            m_timer.restart();
            refresh();
            update();
        }
    });
    slotWindowChanged(window());
}

QmlSpinnyPosition::~QmlSpinnyPosition() {
    QObject::disconnect(m_frameConnection);
}

void QmlSpinnyPosition::setGroup(const QString& group) {
    if (m_group == group) {
        return;
    }

    m_group = group;
    if (m_group.isEmpty()) {
        m_visualPlayPosition.reset();
    } else {
        m_visualPlayPosition = VisualPlayPosition::getVisualPlayPosition(m_group);
    }
    resetPositions();
    emit groupChanged(m_group);
    refresh();
    update();
}

void QmlSpinnyPosition::slotWindowChanged(QQuickWindow* window) {
    QObject::disconnect(m_frameConnection);
    if (window) {
        m_frameConnection = connect(window,
                &QQuickWindow::afterFrameEnd,
                this,
                &QmlSpinnyPosition::slotFrameSwapped);
        if (isVisible()) {
            update();
        }
    }
    m_timer.restart();
}

void QmlSpinnyPosition::slotFrameSwapped() {
    if (!isVisible()) {
        return;
    }

    m_timer.restart();
    refresh();
    update();
}

std::chrono::microseconds QmlSpinnyPosition::fromTimerToNextSync(
        const PerformanceTimer& timer) {
    if (!m_timer.running()) {
        return kSyncInterval;
    }
    return kSyncInterval + std::chrono::microseconds(m_timer.difference(timer).toIntegerMicros());
}

std::chrono::microseconds QmlSpinnyPosition::getSyncInterval() const {
    return kSyncInterval;
}

void QmlSpinnyPosition::refresh() {
    updatePositions();
}

void QmlSpinnyPosition::resetPositions() {
    const bool wasValid = m_valid;
    const bool playPositionWasChanged = m_playPosition != 0.0;
    const bool slipPositionWasChanged = m_slipPosition != 0.0;
    m_valid = false;
    m_playPosition = 0.0;
    m_slipPosition = 0.0;
    if (wasValid) {
        emit validChanged(false);
    }
    if (playPositionWasChanged) {
        emit playPositionChanged(0.0);
    }
    if (slipPositionWasChanged) {
        emit slipPositionChanged(0.0);
    }
}

void QmlSpinnyPosition::updatePositions() {
    if (m_visualPlayPosition.isNull() || !m_visualPlayPosition->isValid()) {
        resetPositions();
        return;
    }

    double playPosition = 0.0;
    double slipPosition = 0.0;
    m_visualPlayPosition->getPlaySlipAtNextVSync(this, &playPosition, &slipPosition);
    if (!std::isfinite(playPosition) || !std::isfinite(slipPosition)) {
        resetPositions();
        return;
    }

    const bool playPositionWasChanged = m_playPosition != playPosition;
    const bool slipPositionWasChanged = m_slipPosition != slipPosition;
    const bool wasValid = m_valid;
    m_playPosition = playPosition;
    m_slipPosition = slipPosition;
    m_valid = true;
    if (!wasValid) {
        emit validChanged(true);
    }
    if (playPositionWasChanged) {
        emit playPositionChanged(m_playPosition);
    }
    if (slipPositionWasChanged) {
        emit slipPositionChanged(m_slipPosition);
    }
}

QSGNode* QmlSpinnyPosition::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) {
    return oldNode;
}

} // namespace qml
} // namespace mixxx
