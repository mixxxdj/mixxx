#pragma once

#include <QHash>
#include <QPainter>
#include <QPixmap>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QString>
#include <QUrl>

#include "library/overviewcache.h"
#include "qmltrackproxy.h"
#include "track/trackid.h"
#include "waveform/waveform.h"

namespace mixxx {
namespace qml {

class QmlWaveformOverview : public QQuickPaintedItem {
    Q_OBJECT
    Q_FLAGS(Channels)
    Q_PROPERTY(mixxx::qml::QmlTrackProxy* track READ getTrack WRITE setTrack
                    NOTIFY trackChanged)
    /// Alternative track identifier for use cases where no `Track`
    /// instance is available (e.g. rows in a browse/external file
    /// table view that point at tracks by location rather than by
    /// `TrackId`). The component operates in one of two mutually
    /// exclusive modes:
    ///
    ///   - `track` mode (e.g. deck): the `track` property is set and
    ///     `trackUrl` is ignored; the waveform is read from the
    ///     `Track`'s in-memory analysis, falling back to the
    ///     `OverviewCache` keyed by `TrackId`.
    ///   - `trackUrl` mode (e.g. table view): the `track` property is
    ///     left `null` and `trackUrl` points at the file's URL; the
    ///     waveform summary is requested from `OverviewCache` keyed
    ///     by the resolved file location.
    ///
    /// Setting both properties at the same time is not supported;
    /// `track` takes precedence when both are set.
    Q_PROPERTY(QUrl trackUrl MEMBER m_trackUrl NOTIFY trackUrlChanged)
    Q_PROPERTY(Channels channels READ getChannels WRITE setChannels NOTIFY channelsChanged)
    Q_PROPERTY(Renderer renderer MEMBER m_renderer NOTIFY rendererChanged)
    Q_PROPERTY(QColor colorHigh MEMBER m_colorHigh NOTIFY colorHighChanged)
    Q_PROPERTY(QColor colorMid MEMBER m_colorMid NOTIFY colorMidChanged)
    Q_PROPERTY(QColor colorLow MEMBER m_colorLow NOTIFY colorLowChanged)
    QML_NAMED_ELEMENT(WaveformOverview)

  public:
    enum class ChannelFlag : int {
        LeftChannel = 1,
        RightChannel = 2,
        BothChannels = LeftChannel | RightChannel,
    };
    Q_DECLARE_FLAGS(Channels, ChannelFlag)

    enum class Renderer {
        RGB = 1,
        Filtered = 2,
    };
    Q_ENUM(Renderer)

    QmlWaveformOverview(QQuickItem* parent = nullptr);
    ~QmlWaveformOverview() override = default;

    void paint(QPainter* painter) override;

    void setTrack(QmlTrackProxy* track);
    QmlTrackProxy* getTrack() const;

    void setChannels(Channels channels);
    Channels getChannels() const;
  private slots:
    void slotWaveformUpdated();
    void slotOverviewChanged(TrackId trackId);
    void slotWaveformSummaryReady(const QObject* pRequester,
            TrackId trackId,
            ConstWaveformPointer pWaveform);
    /// Drives progressive drawing during analysis: the `Track` only
    /// emits `waveformSummaryUpdated` once at the start and once at
    /// the end of analysis, so without this slot the overview would
    /// stay blank (or stale) until the analyzer finishes.
    void slotAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress);

  signals:
    void trackChanged();
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    void channelsChanged(Channels channels);
#else
    void trackUrlChanged();
    void channelsChanged(mixxx::qml::QmlWaveformOverview::Channels channels);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    void rendererChanged(Renderer renderer);
#else
    void rendererChanged(mixxx::qml::QmlWaveformOverview::Renderer renderer);
#endif
    void colorHighChanged(const QColor& color);
    void colorMidChanged(const QColor& color);
    void colorLowChanged(const QColor& color);

  private:
    void drawFiltered(QPainter* pPainter,
            Channels channels,
            ConstWaveformPointer pWaveform,
            int completion) const;
    void drawRgb(QPainter* pPainter,
            Channels channels,
            ConstWaveformPointer pWaveform,
            int completion) const;
    QColor getRgbPenColor(ConstWaveformPointer pWaveform, int completion) const;

    /// Render the waveform overview into a freshly created off-screen
    /// `QPixmap`, populate the global `QPixmapCache` with it and return
    /// it. The caller is responsible for drawing the pixmap onto the
    /// item's painter.
    QPixmap renderWaveformToPixmap(ConstWaveformPointer pWaveform, int completion) const;

    /// Build a unique `QPixmapCache` key for the current state of the
    /// item (track id + geometry + renderer + channels + colors +
    /// waveform completion).
    QString pixmapCacheKey(TrackId trackId, QSize size, int completion) const;

    /// Remove all cached pixmap entries that this item has registered
    /// for `trackId`.
    void invalidatePixmapCache(TrackId trackId);

    /// Invalidate the pixmap cache entries for whichever track is
    /// currently rendered by this item (the `Track` in `track` mode
    /// or the location-resolved track in `trackUrl` mode). Does
    /// nothing if neither mode is active.
    void invalidatePixmapCacheForCurrent();

    QPointer<QmlTrackProxy> m_pTrack;
    QUrl m_trackUrl;
    Channels m_channels;
    Renderer m_renderer;
    QColor m_colorHigh;
    QColor m_colorMid;
    QColor m_colorLow;

    /// Waveform summary loaded via the `trackUrl` path. Only used in
    /// `trackUrl` mode; in `track` mode the waveform is owned by the
    /// `Track` itself.
    ConstWaveformPointer m_waveformSummary;

    /// `TrackId` resolved by `OverviewCache` for the current
    /// `trackUrl`. Used as the `QPixmapCache` key in `trackUrl` mode;
    /// invalid when the location hasn't been resolved yet (or the
    /// file isn't in the library).
    TrackId m_trackIdForUrl;

    OverviewCache* const m_pCache;

    /// Tracks every `QPixmapCache` key inserted by this item keyed by
    /// the track id it relates to, so we can evict all of them when the
    /// waveform for that track is changed/cleared.
    QMultiHash<TrackId, QString> m_cacheKeysByTrackId;
};

} // namespace qml
} // namespace mixxx
