#include "qml/qmlwaveformoverview.h"

#include <QPixmapCache>

#include "moc_qmlwaveformoverview.cpp"
#include "qmlplayerproxy.h"
#include "qmltrackproxy.h"
#include "track/track.h"

namespace {
constexpr double kDesiredChannelHeight = 255;

constexpr const char* kPixmapCachePrefix = "QmlWaveformOverview";
} // namespace

namespace mixxx {
namespace qml {

QmlWaveformOverview::QmlWaveformOverview(QQuickItem* parent)
        : QQuickPaintedItem(parent),
          m_pTrack(nullptr),
          m_channels(ChannelFlag::BothChannels),
          m_pCache(OverviewCache::instance()),
          m_renderer(Renderer::RGB),
          m_colorHigh(0xFF0000),
          m_colorMid(0x00FF00),
          m_colorLow(0x0000FF) {
    if (m_pCache) {
        connect(m_pCache,
                &OverviewCache::overviewChanged,
                this,
                &QmlWaveformOverview::slotOverviewChanged);
        connect(m_pCache,
                &OverviewCache::waveformSummaryReady,
                this,
                &QmlWaveformOverview::slotWaveformSummaryReady);
    }
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
}

void QmlWaveformOverview::slotWaveformUpdated() {
    // The waveform summary changed (cleared/finished/replaced) so any
    // cached pixmap for the current track is now stale.
    if (m_pTrack && m_pTrack->internal()) {
        invalidatePixmapCache(m_pTrack->internal()->getId());
    }
    update();
}

void QmlWaveformOverview::slotOverviewChanged(TrackId trackId) {
    if (m_pTrack && m_pTrack->internal() && m_pTrack->internal()->getId() == trackId) {
        invalidatePixmapCache(trackId);
        update();
    }
}

void QmlWaveformOverview::slotWaveformSummaryReady(const QObject* pRequester,
        TrackId trackId,
        ConstWaveformPointer pWaveform) {
    if (pRequester != this || !pWaveform) {
        return;
    }
    if (!m_pTrack || !m_pTrack->internal()) {
        return;
    }
    TrackPointer pTrack = m_pTrack->internal();
    if (pTrack->getId() != trackId) {
        return;
    }
    // Adopt the loaded summary into the track. `Track::setWaveformSummary`
    // emits `waveformSummaryUpdated`, which `slotWaveformUpdated` listens
    // to, so the item will repaint asynchronously.
    if (!pTrack->getWaveformSummary()) {
        pTrack->setWaveformSummary(pWaveform);
    }
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
        // The track has no analysis loaded in memory. Ask the
        // OverviewCache to fetch it asynchronously from the analysis
        // database. `slotWaveformSummaryReady` will push the result back
        // into the track (which triggers a repaint via
        // `waveformSummaryUpdated`).
        m_pCache->requestWaveformSummary(pTrack->getId(), this);
        return;
    }

    const int dataSize = pWaveform->getDataSize();
    if (dataSize == 0) {
        return;
    }

    constexpr int actualCompletion = 0;
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

    // Try to serve the cached raster pixmap for the current state.
    const QSize size(qRound(width()), qRound(height()));
    if (size.isEmpty()) {
        return;
    }
    const QString cacheKey = pixmapCacheKey(pTrack->getId(), size, waveformCompletion);
    QPixmap pixmap;
    if (QPixmapCache::find(cacheKey, &pixmap)) {
        pPainter->drawPixmap(0, 0, pixmap);
        return;
    }

    pixmap = renderWaveformToPixmap(pWaveform, nextCompletion);
    if (pixmap.isNull()) {
        return;
    }
    QPixmapCache::insert(cacheKey, pixmap);
    m_cacheKeysByTrackId.insert(pTrack->getId(), cacheKey);
    pPainter->drawPixmap(0, 0, pixmap);
}

QPixmap QmlWaveformOverview::renderWaveformToPixmap(
        ConstWaveformPointer pWaveform, int nextCompletion) const {
    constexpr int actualCompletion = 0;

    const QSize size(qRound(width()), qRound(height()));
    if (size.isEmpty()) {
        return QPixmap();
    }

    const qreal desiredWidth = static_cast<qreal>(pWaveform->getDataSize()) / 2;

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter pPainter(&pixmap);

    const Channels channels = m_channels;
    pPainter.save();

    switch (channels) {
    case static_cast<int>(ChannelFlag::LeftChannel):
        // Draw both channels.
        // Set the y axis to half the height of the item
        pPainter.translate(0.0, height());
        // Set the x axis to half the height of the item
        pPainter.scale(width() / desiredWidth, height() / kDesiredChannelHeight);
        break;
    case static_cast<int>(ChannelFlag::RightChannel):
        // Set the x axis to half the height of the item
        pPainter.scale(width() / desiredWidth, height() / kDesiredChannelHeight);
        break;
    default:
        // Draw both channels.
        // Set the y axis to half the height of the item
        pPainter.translate(0.0, height() / 2);
        // Set the x axis to half the height of the item
        pPainter.scale(width() / desiredWidth, height() / (2 * kDesiredChannelHeight));
    }

    const Renderer renderer = m_renderer;
    for (int currentCompletion = actualCompletion;
            currentCompletion < nextCompletion;
            currentCompletion += 2) {
        switch (renderer) {
        case Renderer::Filtered:
            drawFiltered(&pPainter, channels, pWaveform, currentCompletion);
            break;
        default:
            drawRgb(&pPainter, channels, pWaveform, currentCompletion);
        }
    }
    pPainter.restore();
    pPainter.end();
    return pixmap;
}

QString QmlWaveformOverview::pixmapCacheKey(TrackId trackId, QSize size, int completion) const {
    const int channelsValue = static_cast<int>(m_channels);
    const int rendererValue = static_cast<int>(m_renderer);
    return QStringLiteral("%1_%2_%3x%4_%5_%6_%7_%8_%9_%10")
            .arg(QString::fromLatin1(kPixmapCachePrefix),
                    trackId.toString(),
                    QString::number(size.width()),
                    QString::number(size.height()),
                    QString::number(rendererValue),
                    QString::number(channelsValue),
                    QString::number(m_colorHigh.rgba(), 16),
                    QString::number(m_colorMid.rgba(), 16),
                    QString::number(m_colorLow.rgba(), 16))
            .arg(completion);
}

void QmlWaveformOverview::invalidatePixmapCache(TrackId trackId) {
    const auto keys = m_cacheKeysByTrackId.values(trackId);
    for (const QString& key : keys) {
        QPixmapCache::remove(key);
    }
    m_cacheKeysByTrackId.remove(trackId);
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
