#include "skin/qml/qmlwaveformoverview.h"

#include "mixer/basetrackplayer.h"
#include "skin/qml/qmlplayerproxy.h"

namespace {
constexpr double kDesiredHeight = 255 * 2.0;

constexpr qreal lowColorR = 0;
constexpr qreal lowColorG = 0;
constexpr qreal lowColorB = 1;
constexpr qreal midColorR = 0;
constexpr qreal midColorG = 1;
constexpr qreal midColorB = 0;
constexpr qreal highColorR = 1;
constexpr qreal highColorG = 0;
constexpr qreal highColorB = 0;

QColor getPenColor(ConstWaveformPointer pWaveform, int completion) {
    // Retrieve "raw" LMH values from waveform
    qreal low = static_cast<qreal>(pWaveform->getLow(completion));
    qreal mid = static_cast<qreal>(pWaveform->getMid(completion));
    qreal high = static_cast<qreal>(pWaveform->getHigh(completion));

    // Do matrix multiplication
    qreal red = low * lowColorR + mid * midColorR + high * highColorR;
    qreal green = low * lowColorG + mid * midColorG + high * highColorG;
    qreal blue = low * lowColorB + mid * midColorB + high * highColorB;

    // Normalize and draw
    qreal max = math_max3(red, green, blue);
    if (max > 0.0) {
        QColor color;
        color.setRgbF(red / max, green / max, blue / max);
        return color;
    }
    return QColor();
}
} // namespace

namespace mixxx {
namespace skin {
namespace qml {

QmlWaveformOverview::QmlWaveformOverview(QQuickItem* parent)
        : QQuickPaintedItem(parent), m_pPlayer(nullptr) {
}

QmlPlayerProxy* QmlWaveformOverview::getPlayer() const {
    return m_pPlayer;
}

void QmlWaveformOverview::setPlayer(QmlPlayerProxy* pPlayer) {
    if (m_pPlayer == pPlayer) {
        return;
    }

    if (m_pPlayer != nullptr) {
        disconnect(m_pPlayer->internalTrackPlayer(), nullptr, this, nullptr);
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

    pPainter->save();
    // Set the y axis to half the height of the item
    pPainter->translate(0.0, height() / 2);
    // Set the x axis to half the height of the item
    pPainter->scale(width() / desiredWidth, height() / kDesiredHeight);

    for (int currentCompletion = actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        const double offsetX = currentCompletion / 2.0;

        // Draw left channel
        const QColor leftColor = getPenColor(pWaveform, currentCompletion);
        if (leftColor.isValid()) {
            const uint8_t leftValue = pWaveform->getAll(currentCompletion);
            pPainter->setPen(leftColor);
            pPainter->drawLine(QPointF(offsetX, -leftValue), QPointF(offsetX, 0.0));
        }

        // Draw right channel
        QColor rightColor = getPenColor(pWaveform, currentCompletion + 1);
        if (rightColor.isValid()) {
            const uint8_t rightValue = pWaveform->getAll(currentCompletion + 1);
            pPainter->setPen(rightColor);
            pPainter->drawLine(QPointF(offsetX, 0.0), QPointF(offsetX, rightValue));
        }
    }
    pPainter->restore();
}

} // namespace qml
} // namespace skin
} // namespace mixxx
