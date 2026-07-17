#pragma once

#include <QHash>
#include <QPainter>
#include <QPixmap>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QString>

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
                    NOTIFY trackChanged REQUIRED)
    Q_PROPERTY(mixxx::qml::QmlWaveformOverview::Channels channels READ
                    getChannels WRITE setChannels NOTIFY channelsChanged)
    Q_PROPERTY(mixxx::qml::QmlWaveformOverview::Renderer renderer MEMBER
                    m_renderer NOTIFY rendererChanged)
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

  signals:
    void trackChanged();
    void channelsChanged(mixxx::qml::QmlWaveformOverview::Channels channels);
    void rendererChanged(mixxx::qml::QmlWaveformOverview::Renderer renderer);
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

    QPointer<QmlTrackProxy> m_pTrack;
    Channels m_channels;
    Renderer m_renderer;
    QColor m_colorHigh;
    QColor m_colorMid;
    QColor m_colorLow;

    OverviewCache* const m_pCache;

    /// Tracks every `QPixmapCache` key inserted by this item keyed by
    /// the track id it relates to, so we can evict all of them when the
    /// waveform for that track is changed/cleared.
    QMultiHash<TrackId, QString> m_cacheKeysByTrackId;
};

} // namespace qml
} // namespace mixxx
