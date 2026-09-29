#include "qml/qmlvinylsignalquality.h"

#include <QColor>
#include <QPainter>

#include "moc_qmlvinylsignalquality.cpp"
#include "qml/qmlapplicationproxy.h"
#include "vinylcontrol/vinylcontrolmanager.h"

namespace mixxx {
namespace qml {

QmlVinylSignalQuality::QmlVinylSignalQuality(QQuickItem* parent)
        : QQuickPaintedItem(parent),
          m_signalImage(MIXXX_VINYL_SCOPE_SIZE,
                  MIXXX_VINYL_SCOPE_SIZE,
                  QImage::Format_ARGB32) {
    connect(this, &QQuickItem::visibleChanged, this, [this]() {
        updateSignalQualityListener();
    });
}

QmlVinylSignalQuality::~QmlVinylSignalQuality() {
#ifdef __VINYLCONTROL__
    if (m_listenerRegistered && m_pVinylControlManager) {
        m_pVinylControlManager->removeSignalQualityListener(this);
    }
#endif
}

void QmlVinylSignalQuality::setGroup(const QString& group) {
    if (m_group == group) {
        return;
    }

    m_group = group;
    emit groupChanged(m_group);
    updateSignalQualityListener();
}

void QmlVinylSignalQuality::setActive(bool active) {
    if (m_active == active) {
        return;
    }

    m_active = active;
    emit activeChanged();
    updateSignalQualityListener();
}

void QmlVinylSignalQuality::updateSignalQualityListener() {
    VinylControlManager* pVinylControlManager =
            QmlApplicationProxy::vinylControlManager();
    int vinylInput = -1;
#ifdef __VINYLCONTROL__
    if (pVinylControlManager) {
        vinylInput = pVinylControlManager->vinylInputFromGroup(m_group);
    }
#endif

    const bool shouldListen = m_active && isVisible() &&
            pVinylControlManager != nullptr && vinylInput >= 0;
    const bool listenerNeedsUpdate = m_listenerRegistered &&
            (!shouldListen ||
                    m_pVinylControlManager.data() != pVinylControlManager ||
                    m_vinylInput != vinylInput);
    if (listenerNeedsUpdate) {
#ifdef __VINYLCONTROL__
        if (m_pVinylControlManager) {
            m_pVinylControlManager->removeSignalQualityListener(this);
        }
#endif
        m_listenerRegistered = false;
        m_hasReport = false;
        update();
    }

    m_pVinylControlManager = pVinylControlManager;
    m_vinylInput = vinylInput;

#ifdef __VINYLCONTROL__
    if (shouldListen && !m_listenerRegistered) {
        pVinylControlManager->addSignalQualityListener(this);
        m_listenerRegistered = true;
    }
#endif
}

void QmlVinylSignalQuality::onVinylSignalQualityUpdate(
        const VinylSignalQualityReport& report) {
#ifdef __VINYLCONTROL__
    if (!m_active || !m_listenerRegistered || !m_pVinylControlManager ||
            report.processor != m_vinylInput) {
        return;
    }

    QColor qualityColor;
    qualityColor.setHsv(static_cast<int>(120.0 * report.timecode_quality), 255, 255);
    int red;
    int green;
    int blue;
    qualityColor.getRgb(&red, &green, &blue);

    for (int y = 0; y < MIXXX_VINYL_SCOPE_SIZE; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(m_signalImage.scanLine(y));
        for (int x = 0; x < MIXXX_VINYL_SCOPE_SIZE; ++x) {
            *line = qRgba(red,
                    green,
                    blue,
                    static_cast<int>(
                            report.scope[x + MIXXX_VINYL_SCOPE_SIZE * y] * 0.75));
            ++line;
        }
    }

    m_hasReport = true;
    update();
#else
    Q_UNUSED(report);
#endif
}

void QmlVinylSignalQuality::paint(QPainter* painter) {
    if (!m_hasReport) {
        return;
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawImage(boundingRect(), m_signalImage, m_signalImage.rect());
}

} // namespace qml
} // namespace mixxx
