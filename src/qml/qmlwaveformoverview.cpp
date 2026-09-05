#include "qml/qmlwaveformoverview.h"

#include <algorithm>

#include "moc_qmlwaveformoverview.cpp"
#include "qmlplayerproxy.h"
#include "qmltrackproxy.h"
#include "track/track.h"

namespace {
constexpr double kDesiredChannelHeight = 255;
constexpr int kWaveformImageHeight = 2 * static_cast<int>(kDesiredChannelHeight);
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
          m_colorLow(0x0000FF) {
    const auto invalidate = [this] {
        invalidateWaveformCache();
        update();
    };
    connect(this, &QmlWaveformOverview::rendererChanged, this, invalidate);
    connect(this, &QmlWaveformOverview::colorHighChanged, this, invalidate);
    connect(this, &QmlWaveformOverview::colorMidChanged, this, invalidate);
    connect(this, &QmlWaveformOverview::colorLowChanged, this, invalidate);
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
    invalidateWaveformCache();

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
    update();
}

double QmlWaveformOverview::analyzerProgress() const {
    return m_analyzerProgress;
}

void QmlWaveformOverview::setAnalyzerProgress(double analyzerProgress) {
    if (m_analyzerProgress == analyzerProgress) {
        return;
    }
    m_analyzerProgress = analyzerProgress;
    emit analyzerProgressChanged();
    // Analyzer progress is the legacy update source for partial overview data.
    // The next paint only appends samples that became available since the last one.
    update();
}

void QmlWaveformOverview::slotWaveformUpdated() {
    update();
}

void QmlWaveformOverview::paint(QPainter* pPainter) {
    if (!m_pTrack) {
        return;
    }
    TrackPointer pTrack = m_pTrack->internal();
    if (!pTrack) {
        return;
    }

    ConstWaveformPointer pWaveform = pTrack->getWaveformSummary();
    if (!pWaveform) {
        return;
    }

    if (!drawNextWaveformPart(pWaveform) || m_waveformImage.isNull()) {
        return;
    }

    QRectF sourceRect(0, 0, m_waveformImage.width(), m_waveformImage.height());
    if (m_channels == static_cast<int>(ChannelFlag::LeftChannel)) {
        sourceRect.setHeight(kDesiredChannelHeight);
    } else if (m_channels == static_cast<int>(ChannelFlag::RightChannel)) {
        sourceRect.setTop(kDesiredChannelHeight);
        sourceRect.setHeight(kDesiredChannelHeight);
    }
    pPainter->drawImage(boundingRect(), m_waveformImage, sourceRect);
}

bool QmlWaveformOverview::drawNextWaveformPart(ConstWaveformPointer pWaveform) {
    const int dataSize = pWaveform->getDataSize();
    if (dataSize <= 0) {
        return false;
    }

    const int imageWidth = std::max(1, dataSize / 2);
    if (m_cachedWaveform != pWaveform || m_waveformImage.width() != imageWidth) {
        m_cachedWaveform = pWaveform;
        m_waveformImage = QImage(
                imageWidth, kWaveformImageHeight, QImage::Format_ARGB32_Premultiplied);
        m_waveformImage.fill(Qt::transparent);
        m_actualCompletion = 0;
    }

    const int waveformCompletion = std::clamp(pWaveform->getCompletion(), 0, dataSize) & ~1;
    if (waveformCompletion < m_actualCompletion) {
        m_waveformImage.fill(Qt::transparent);
        m_actualCompletion = 0;
    }
    if (waveformCompletion <= m_actualCompletion) {
        return !m_waveformImage.isNull();
    }

    QPainter painter(&m_waveformImage);
    painter.translate(0.0, kDesiredChannelHeight);
    for (int currentCompletion = m_actualCompletion;
            currentCompletion < waveformCompletion;
            currentCompletion += 2) {
        switch (m_renderer) {
        case Renderer::Filtered:
            drawFiltered(&painter, m_channels, pWaveform, currentCompletion);
            break;
        default:
            drawRgb(&painter, m_channels, pWaveform, currentCompletion);
        }
    }
    m_actualCompletion = waveformCompletion;
    return true;
}

void QmlWaveformOverview::invalidateWaveformCache() {
    m_cachedWaveform.clear();
    m_waveformImage = QImage();
    m_actualCompletion = 0;
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

    if (channels.testFlag(ChannelFlag::LeftChannel)) {
        const uint8_t leftHigh = pWaveform->getHigh(completion);
        pPainter->setPen(m_colorHigh);
        pPainter->drawLine(QPointF(offsetX, 2 * -leftHigh), QPointF(offsetX, 0.0));

        const uint8_t leftMid = pWaveform->getMid(completion);
        pPainter->setPen(m_colorMid);
        pPainter->drawLine(QPointF(offsetX, 1.5 * -leftMid), QPointF(offsetX, 0.0));

        const uint8_t leftLow = pWaveform->getLow(completion);
        pPainter->setPen(m_colorLow);
        pPainter->drawLine(QPointF(offsetX, -leftLow), QPointF(offsetX, 0.0));
    }

    if (channels.testFlag(ChannelFlag::RightChannel)) {
        const uint8_t rightHigh = pWaveform->getHigh(completion + 1);
        pPainter->setPen(m_colorHigh);
        pPainter->drawLine(QPointF(offsetX, 0), QPointF(offsetX, 2 * rightHigh));

        const uint8_t rightMid = pWaveform->getMid(completion + 1) * 2;
        pPainter->setPen(m_colorMid);
        pPainter->drawLine(QPointF(offsetX, 0), QPointF(offsetX, 1.5 * rightMid));

        const uint8_t rightLow = pWaveform->getLow(completion + 1);
        pPainter->setPen(m_colorLow);
        pPainter->drawLine(QPointF(offsetX, 0), QPointF(offsetX, rightLow));
    }
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
