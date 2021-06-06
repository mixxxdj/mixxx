#include "skin/qml/qmlwaveformoverview.h"

#include "mixer/basetrackplayer.h"
#include "skin/qml/qmlplayerproxy.h"

namespace {
constexpr double kDesiredChannelHeight = 255;
} // namespace

namespace mixxx {
namespace skin {
namespace qml {

QmlWaveformOverview::QmlWaveformOverview(QQuickItem* parent)
        : QQuickPaintedItem(parent),
          m_pPlayer(nullptr),
          m_channels(ChannelFlag::BothChannels),
          m_renderer(Renderer::RGB),
          m_colorHigh(0xFF0000),
          m_colorMid(0x00FF00),
          m_colorLow(0x0000FF) {
}

QmlPlayerProxy* QmlWaveformOverview::getPlayer() const {
    return m_pPlayer;
}

void QmlWaveformOverview::setPlayer(QmlPlayerProxy* pPlayer) {
    if (m_pPlayer == pPlayer) {
        return;
    }

    if (m_pPlayer != nullptr) {
        m_pPlayer->internalTrackPlayer()->disconnect(this);
    }

    m_pPlayer = pPlayer;

    if (pPlayer != nullptr) {
        connect(m_pPlayer->internalTrackPlayer(),
                &BaseTrackPlayer::newTrackLoaded,
                this,
                &QmlWaveformOverview::slotTrackLoaded);
        connect(m_pPlayer->internalTrackPlayer(),
                &BaseTrackPlayer::loadingTrack,
                this,
                &QmlWaveformOverview::slotTrackLoading);
        connect(m_pPlayer->internalTrackPlayer(),
                &BaseTrackPlayer::playerEmpty,
                this,
                &QmlWaveformOverview::slotTrackUnloaded);
    }

    emit playerChanged();
    update();
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
}

void QmlWaveformOverview::slotTrackLoaded(TrackPointer pTrack) {
    // TODO: Investigate if it's a bug that this debug assertion fails when
    // passing tracks on the command line
    // DEBUG_ASSERT(m_pCurrentTrack == pTrack);
    setCurrentTrack(pTrack);
}

void QmlWaveformOverview::slotTrackLoading(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pOldTrack); // only used in DEBUG_ASSERT
    DEBUG_ASSERT(m_pCurrentTrack == pOldTrack);
    setCurrentTrack(pNewTrack);
}

void QmlWaveformOverview::slotTrackUnloaded() {
    setCurrentTrack(nullptr);
}

void QmlWaveformOverview::setCurrentTrack(TrackPointer pTrack) {
    // TODO: Check if this is actually possible
    if (m_pCurrentTrack == pTrack) {
        return;
    }

    if (m_pCurrentTrack != nullptr) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }

    m_pCurrentTrack = pTrack;
    if (pTrack != nullptr) {
        connect(pTrack.get(),
                &Track::waveformSummaryUpdated,
                this,
                &QmlWaveformOverview::slotWaveformUpdated);
    }
    slotWaveformUpdated();
}

void QmlWaveformOverview::slotWaveformUpdated() {
    update();
}

void QmlWaveformOverview::paint(QPainter* pPainter) {
    TrackPointer pTrack = m_pCurrentTrack;
    if (!pTrack) {
        return;
    }

    ConstWaveformPointer pWaveform = pTrack->getWaveformSummary();
    if (!pWaveform) {
        return;
    }

    const int dataSize = pWaveform->getDataSize();
    if (dataSize == 0) {
        return;
    }

    const int actualCompletion = 0;
    // Always multiple of 2
    const int waveformCompletion = pWaveform->getCompletion();
    // Test if there is some new to draw (at least of pixel width)
    const int completionIncrement = waveformCompletion - actualCompletion;

    const qreal desiredWidth = static_cast<qreal>(dataSize) / 2;
    const double visiblePixelIncrement = completionIncrement * desiredWidth / dataSize;
    if (waveformCompletion < (dataSize - 2) &&
            (completionIncrement < 2 || visiblePixelIncrement == 0)) {
        return;
    }

    const int nextCompletion = actualCompletion + completionIncrement;

    const Channels channels = m_channels;
    pPainter->save();

    switch (channels) {
    case static_cast<int>(ChannelFlag::LeftChannel):
        // Draw both channels.
        // Set the y axis to half the height of the item
        pPainter->translate(0.0, height());
        // Set the x axis to half the height of the item
        pPainter->scale(width() / desiredWidth, height() / kDesiredChannelHeight);
        break;
    case static_cast<int>(ChannelFlag::RightChannel):
        // Set the x axis to half the height of the item
        pPainter->scale(width() / desiredWidth, height() / kDesiredChannelHeight);
        break;
    default:
        // Draw both channels.
        // Set the y axis to half the height of the item
        pPainter->translate(0.0, height() / 2);
        // Set the x axis to half the height of the item
        pPainter->scale(width() / desiredWidth, height() / (2 * kDesiredChannelHeight));
    }

    Renderer renderer = m_renderer;
    for (int currentCompletion = actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        switch (renderer) {
        case Renderer::Filtered:
            drawFiltered(pPainter, channels, pWaveform, currentCompletion);
            break;
        default:
            drawRgb(pPainter, channels, pWaveform, currentCompletion);
        }
    }
    pPainter->restore();
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
        color.setRgbF(red / max, green / max, blue / max);
        return color;
    }
    return QColor();
}

} // namespace qml
} // namespace skin
} // namespace mixxx
