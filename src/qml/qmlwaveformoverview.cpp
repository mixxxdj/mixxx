#include "qml/qmlwaveformoverview.h"

#include <QMutexLocker>
#include <algorithm>

#include "moc_qmlwaveformoverview.cpp"
#include "qmlplayerproxy.h"
#include "qmltrackproxy.h"
#include "track/track.h"

namespace {
constexpr double kDesiredChannelHeight = 255;
constexpr int kWaveformImageHeight = 2 * static_cast<int>(kDesiredChannelHeight);
constexpr int kWaveformProgressUpdateIntervalMs = 60;
} // namespace

namespace mixxx {
namespace qml {

QmlWaveformOverview::QmlWaveformOverview(QQuickItem* parent)
        : QQuickPaintedItem(parent),
          m_pTrack(nullptr),
          m_channels(ChannelFlag::BothChannels),
          m_renderer(Renderer::RGB),
          m_colorHigh(0xFF0000),
          m_colorMid(0x00FF00),
          m_colorLow(0x0000FF),
          m_waveformProgressTimer() {
    m_waveformProgressTimer.setInterval(kWaveformProgressUpdateIntervalMs);
    connect(&m_waveformProgressTimer,
            &QTimer::timeout,
            this,
            &QmlWaveformOverview::slotWaveformUpdated);
    connect(this, &QmlWaveformOverview::rendererChanged, this, [this] {
        invalidateWaveformCache();
        slotWaveformUpdated();
    });
    connect(this, &QmlWaveformOverview::colorHighChanged, this, [this](const QColor&) {
        invalidateWaveformCache();
        slotWaveformUpdated();
    });
    connect(this, &QmlWaveformOverview::colorMidChanged, this, [this](const QColor&) {
        invalidateWaveformCache();
        slotWaveformUpdated();
    });
    connect(this, &QmlWaveformOverview::colorLowChanged, this, [this](const QColor&) {
        invalidateWaveformCache();
        slotWaveformUpdated();
    });
    connect(this, &QmlWaveformOverview::normalizedChanged, this, &QQuickItem::update);
}

QmlTrackProxy* QmlWaveformOverview::getTrack() const {
    return m_pTrack;
}

void QmlWaveformOverview::setTrack(QmlTrackProxy* pTrack) {
    if (m_pTrack == pTrack) {
        return;
    }

    if (m_pTrack != nullptr && m_pTrack->internal() != nullptr) {
        m_pTrack->internal()->disconnect(this);
    }

    m_pTrack = pTrack;

    m_waveformProgressTimer.stop();
    {
        const QMutexLocker locker(&m_waveformCacheMutex);
        m_cachedWaveform.clear();
        m_waveformImage = QImage();
        m_actualCompletion = 0;
        m_waveformPeak = 1.0f;
    }

    if (m_pTrack != nullptr && pTrack->internal() != nullptr) {
        connect(pTrack->internal().get(),
                &Track::waveformSummaryUpdated,
                this,
                &QmlWaveformOverview::slotWaveformUpdated);
    }
    slotWaveformUpdated();
}

QmlWaveformOverview::Channels QmlWaveformOverview::getChannels() const {
    return m_channels;
}

void QmlWaveformOverview::setChannels(QmlWaveformOverview::Channels channels) {
    if (m_channels == channels) {
        return;
    }

    m_channels = channels;
    emit channelsChanged(channels);
    invalidateWaveformCache();
    slotWaveformUpdated();
}

void QmlWaveformOverview::slotWaveformUpdated() {
    if (!m_pTrack || !m_pTrack->internal()) {
        invalidateWaveformCache();
        m_waveformProgressTimer.stop();
        update();
        return;
    }

    const ConstWaveformPointer pWaveform = m_pTrack->internal()->getWaveformSummary();
    if (!pWaveform) {
        invalidateWaveformCache();
        m_cachedWaveform.clear();
        m_waveformProgressTimer.stop();
        update();
        return;
    }

    const QMutexLocker locker(&m_waveformCacheMutex);
    const bool cacheReset = pWaveform != m_cachedWaveform ||
            m_waveformImage.isNull() ||
            m_waveformImage.width() != pWaveform->getDataSize() / 2 + 1;
    if (cacheReset) {
        m_cachedWaveform = pWaveform;
        m_waveformImage = QImage(pWaveform->getDataSize() / 2 + 1,
                kWaveformImageHeight,
                QImage::Format_ARGB32_Premultiplied);
        m_waveformImage.fill(Qt::transparent);
        m_actualCompletion = 0;
        m_waveformPeak = 1.0f;
    }

    const bool waveformChanged = drawNextWaveformPart();
    if (pWaveform->getCompletion() < pWaveform->getDataSize()) {
        if (!m_waveformProgressTimer.isActive()) {
            m_waveformProgressTimer.start();
        }
    } else {
        m_waveformProgressTimer.stop();
    }

    if (cacheReset || waveformChanged) {
        update();
    }
}

void QmlWaveformOverview::paint(QPainter* pPainter) {
    const QMutexLocker locker(&m_waveformCacheMutex);
    if (!m_pTrack || m_waveformImage.isNull()) {
        return;
    }
    const QRectF targetRect = boundingRect();
    QRectF sourceRect(0, 0, m_waveformImage.width(), m_waveformImage.height());
    if (m_channels == static_cast<int>(ChannelFlag::LeftChannel)) {
        sourceRect.setHeight(kDesiredChannelHeight);
    } else if (m_channels == static_cast<int>(ChannelFlag::RightChannel)) {
        sourceRect.setTop(kDesiredChannelHeight);
        sourceRect.setHeight(kDesiredChannelHeight);
    }
    if (m_normalized && m_actualCompletion >= m_cachedWaveform->getDataSize() - 2 &&
            m_waveformPeak > 1.0f) {
        const int diffGain = std::clamp(
                static_cast<int>(255.0f - m_waveformPeak - 1.0f),
                0,
                static_cast<int>((m_waveformImage.height() - 1) / 2));
        sourceRect.adjust(0, diffGain, 0, -diffGain);
    }
    pPainter->drawImage(targetRect, m_waveformImage, sourceRect);
}

void QmlWaveformOverview::invalidateWaveformCache() {
    const QMutexLocker locker(&m_waveformCacheMutex);
    m_actualCompletion = 0;
    m_waveformPeak = 1.0f;
    if (!m_waveformImage.isNull()) {
        m_waveformImage.fill(Qt::transparent);
    }
}

bool QmlWaveformOverview::drawNextWaveformPart() {
    if (!m_cachedWaveform || m_waveformImage.isNull()) {
        return false;
    }

    const int dataSize = m_cachedWaveform->getDataSize();
    if (dataSize <= 0) {
        return false;
    }

    const int waveformCompletion = std::min(m_cachedWaveform->getCompletion(), dataSize);
    const int completionIncrement = waveformCompletion - m_actualCompletion;
    const double visiblePixelIncrement =
            completionIncrement * width() / static_cast<double>(dataSize);
    if (waveformCompletion < dataSize - 2 &&
            (completionIncrement < 2 || visiblePixelIncrement == 0)) {
        return false;
    }

    QPainter painter(&m_waveformImage);
    painter.translate(0.0, kDesiredChannelHeight);
    for (int currentCompletion = m_actualCompletion;
            currentCompletion < waveformCompletion;
            currentCompletion += 2) {
        m_waveformPeak = std::max(
                m_waveformPeak,
                std::max(static_cast<float>(m_cachedWaveform->getAll(currentCompletion)),
                        static_cast<float>(m_cachedWaveform->getAll(currentCompletion + 1))));
        switch (m_renderer) {
        case Renderer::Filtered:
            drawFiltered(&painter, m_channels, m_cachedWaveform, currentCompletion);
            break;
        default:
            drawRgb(&painter, m_channels, m_cachedWaveform, currentCompletion);
        }
    }
    m_actualCompletion = waveformCompletion;
    return completionIncrement > 0;
}

void QmlWaveformOverview::drawRgb(QPainter* pPainter,
        Channels channels,
        ConstWaveformPointer pWaveform,
        int completion) const {
    const double offsetX = completion / 2.0;

    if (channels.testFlag(ChannelFlag::LeftChannel)) {
        // Draw left channel
        const QColor leftColor = getRgbPenColor(pWaveform, completion);
        if (leftColor.isValid()) {
            const uint8_t leftValue = pWaveform->getAll(completion);
            pPainter->setPen(leftColor);
            pPainter->drawLine(QPointF(offsetX, -leftValue), QPointF(offsetX, 0.0));
        }
    }

    if (channels.testFlag(ChannelFlag::RightChannel)) {
        // Draw right channel
        QColor rightColor = getRgbPenColor(pWaveform, completion + 1);
        if (rightColor.isValid()) {
            const uint8_t rightValue = pWaveform->getAll(completion + 1);
            pPainter->setPen(rightColor);
            pPainter->drawLine(QPointF(offsetX, 0.0), QPointF(offsetX, rightValue));
        }
    }
}

void QmlWaveformOverview::drawFiltered(QPainter* pPainter,
        Channels channels,
        ConstWaveformPointer pWaveform,
        int completion) const {
    const double offsetX = completion / 2.0;

    const auto drawBand = [pPainter, offsetX, channels](const QColor& color,
                                  uint8_t leftValue,
                                  uint8_t rightValue) {
        pPainter->setPen(color);
        pPainter->drawLine(QPointF(offsetX,
                                   channels.testFlag(ChannelFlag::LeftChannel)
                                           ? -leftValue
                                           : 0),
                QPointF(offsetX,
                        channels.testFlag(ChannelFlag::RightChannel) ? rightValue : 0));
    };
    drawBand(m_colorLow,
            pWaveform->getLow(completion),
            pWaveform->getLow(completion + 1));
    drawBand(m_colorMid,
            pWaveform->getMid(completion),
            pWaveform->getMid(completion + 1));
    drawBand(m_colorHigh,
            pWaveform->getHigh(completion),
            pWaveform->getHigh(completion + 1));
}

QColor QmlWaveformOverview::getRgbPenColor(ConstWaveformPointer pWaveform, int completion) const {
    // Retrieve "raw" LMH values from waveform
    qreal low = static_cast<qreal>(pWaveform->getLow(completion));
    qreal mid = static_cast<qreal>(pWaveform->getMid(completion));
    qreal high = static_cast<qreal>(pWaveform->getHigh(completion));

    // Do matrix multiplication
    qreal red = low * m_colorLow.redF() + mid * m_colorMid.redF() + high * m_colorHigh.redF();
    qreal green = low * m_colorLow.greenF() + mid * m_colorMid.greenF() +
            high * m_colorHigh.greenF();
    qreal blue = low * m_colorLow.blueF() + mid * m_colorMid.blueF() + high * m_colorHigh.blueF();

    // Normalize and draw
    qreal max = math_max3(red, green, blue);
    if (max > 0.0) {
        QColor color;
        color.setRgbF(
                static_cast<float>(red / max),
                static_cast<float>(green / max),
                static_cast<float>(blue / max));
        return color;
    }
    return QColor();
}

} // namespace qml
} // namespace mixxx
