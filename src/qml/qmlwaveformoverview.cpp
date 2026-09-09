#include "qml/qmlwaveformoverview.h"

#include <algorithm>

#include "moc_qmlwaveformoverview.cpp"
#include "qmltrackproxy.h"
#include "track/track.h"
#include "util/math.h"

namespace {
constexpr double kDesiredChannelHeight = 255;
} // namespace

namespace mixxx {
namespace qml {

QmlWaveformOverview::QmlWaveformOverview(QQuickItem* parent)
        : QQuickPaintedItem(parent),
          m_pTrack(nullptr),
          m_channels(ChannelFlag::BothChannels),
          m_renderer(Renderer::RGB),
          m_stereo(true),
          m_normalized(false),
          m_minuteMarkers(true),
          m_colorHigh(0xFF0000),
          m_colorMid(0x00FF00),
          m_colorLow(0x0000FF) {
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
    update();
}

void QmlWaveformOverview::setRenderer(Renderer renderer) {
    if (m_renderer == renderer) {
        return;
    }
    m_renderer = renderer;
    emit rendererChanged(renderer);
    update();
}

void QmlWaveformOverview::setStereo(bool stereo) {
    if (m_stereo == stereo) {
        return;
    }
    m_stereo = stereo;
    emit stereoChanged();
    update();
}

void QmlWaveformOverview::setNormalized(bool normalized) {
    if (m_normalized == normalized) {
        return;
    }
    m_normalized = normalized;
    emit normalizedChanged();
    update();
}

void QmlWaveformOverview::setMinuteMarkers(bool minuteMarkers) {
    if (m_minuteMarkers == minuteMarkers) {
        return;
    }
    m_minuteMarkers = minuteMarkers;
    emit minuteMarkersChanged();
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

    const int dataSize = pWaveform->getDataSize();
    if (dataSize <= 0) {
        return;
    }

    const int waveformCompletion =
            std::clamp(pWaveform->getCompletion(), 0, dataSize) & ~1;
    if (waveformCompletion <= 0) {
        return;
    }

    const double desiredWidth = static_cast<double>(dataSize) / 2.0;
    double amplitudeScale = 1.0;
    if (m_normalized) {
        unsigned char peak = 0;
        for (int i = 0; i < waveformCompletion; ++i) {
            peak = std::max(peak, pWaveform->getAll(i));
        }
        if (peak > 0) {
            amplitudeScale = 255.0 / peak;
        }
    }

    pPainter->save();
    if (!m_stereo) {
        pPainter->translate(0.0, height());
        pPainter->scale(width() / desiredWidth,
                -height() / (2.0 * kDesiredChannelHeight) * amplitudeScale);
    } else {
        switch (static_cast<int>(m_channels)) {
        case static_cast<int>(ChannelFlag::LeftChannel):
            pPainter->translate(0.0, height());
            pPainter->scale(width() / desiredWidth,
                    height() / kDesiredChannelHeight * amplitudeScale);
            break;
        case static_cast<int>(ChannelFlag::RightChannel):
            pPainter->scale(width() / desiredWidth,
                    height() / kDesiredChannelHeight * amplitudeScale);
            break;
        default:
            pPainter->translate(0.0, height() / 2.0);
            pPainter->scale(width() / desiredWidth,
                    height() / (2.0 * kDesiredChannelHeight) * amplitudeScale);
            break;
        }
    }

    for (int currentCompletion = 0;
            currentCompletion < waveformCompletion;
            currentCompletion += 2) {
        switch (m_renderer) {
        case Renderer::Filtered:
            drawFiltered(pPainter, m_channels, pWaveform, currentCompletion);
            break;
        case Renderer::HSV:
            drawHsv(pPainter, m_channels, pWaveform, currentCompletion);
            break;
        default:
            drawRgb(pPainter, m_channels, pWaveform, currentCompletion);
        }
    }
    pPainter->restore();

    if (m_minuteMarkers) {
        drawMinuteMarkers(pPainter, m_pTrack->getDuration());
    }
}

void QmlWaveformOverview::drawRgb(QPainter* pPainter,
        Channels channels,
        ConstWaveformPointer pWaveform,
        int completion) const {
    const double offsetX = completion / 2.0;

    if (!m_stereo) {
        const uint8_t leftValue = pWaveform->getAll(completion);
        const uint8_t rightValue = pWaveform->getAll(completion + 1);
        const QColor color = getRgbPenColor(pWaveform, completion);
        if (color.isValid()) {
            pPainter->setPen(color);
            pPainter->drawLine(QPointF(offsetX, 0),
                    QPointF(offsetX, leftValue + rightValue));
        }
        return;
    }

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

    if (!m_stereo) {
        const uint8_t leftHigh = pWaveform->getHigh(completion);
        const uint8_t rightHigh = pWaveform->getHigh(completion + 1);
        const uint8_t leftMid = pWaveform->getMid(completion);
        const uint8_t rightMid = pWaveform->getMid(completion + 1);
        const uint8_t leftLow = pWaveform->getLow(completion);
        const uint8_t rightLow = pWaveform->getLow(completion + 1);
        pPainter->setPen(m_colorHigh);
        pPainter->drawLine(QPointF(offsetX, 0),
                QPointF(offsetX, 2 * (leftHigh + rightHigh)));
        pPainter->setPen(m_colorMid);
        pPainter->drawLine(QPointF(offsetX, 0),
                QPointF(offsetX, 1.5 * (leftMid + rightMid)));
        pPainter->setPen(m_colorLow);
        pPainter->drawLine(QPointF(offsetX, 0),
                QPointF(offsetX, leftLow + rightLow));
        return;
    }

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

void QmlWaveformOverview::drawHsv(QPainter* pPainter,
        Channels channels,
        ConstWaveformPointer pWaveform,
        int completion) const {
    const double offsetX = completion / 2.0;
    float hue = 0;
    float saturation = 0;
    float value = 0;
    m_colorLow.getHsvF(&hue, &saturation, &value);
    const int leftAll = pWaveform->getAll(completion);
    const int rightAll = pWaveform->getAll(completion + 1);
    const int leftLow = pWaveform->getLow(completion);
    const int rightLow = pWaveform->getLow(completion + 1);
    const int leftHigh = pWaveform->getHigh(completion);
    const int rightHigh = pWaveform->getHigh(completion + 1);
    const int total = leftLow + rightLow + pWaveform->getMid(completion) +
            pWaveform->getMid(completion + 1) + leftHigh + rightHigh;
    if (total == 0) {
        return;
    }
    QColor color;
    color.setHsvF(hue,
            1.0f - static_cast<float>(leftHigh + rightHigh) / (1.2f * total),
            1.0f - static_cast<float>(leftLow + rightLow) / (1.2f * total));

    pPainter->setPen(color);
    if (!m_stereo) {
        pPainter->drawLine(QPointF(offsetX, 0), QPointF(offsetX, leftAll + rightAll));
    } else {
        if (channels.testFlag(ChannelFlag::LeftChannel)) {
            pPainter->drawLine(QPointF(offsetX, -leftAll), QPointF(offsetX, 0));
        }
        if (channels.testFlag(ChannelFlag::RightChannel)) {
            pPainter->drawLine(QPointF(offsetX, 0), QPointF(offsetX, rightAll));
        }
    }
}

void QmlWaveformOverview::drawMinuteMarkers(QPainter* pPainter, double duration) const {
    if (duration <= 60.0 || width() <= 0) {
        return;
    }
    pPainter->save();
    pPainter->setPen(QPen(QColor(245, 245, 245, 180), 1));
    const double markerHeight = height() * 0.08;
    for (double seconds = 60.0; seconds < duration; seconds += 60.0) {
        const double x = width() * seconds / duration;
        pPainter->drawLine(QPointF(x, 0), QPointF(x, markerHeight));
        if (m_stereo) {
            pPainter->drawLine(QPointF(x, height() - markerHeight),
                    QPointF(x, height()));
        }
    }
    pPainter->restore();
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
