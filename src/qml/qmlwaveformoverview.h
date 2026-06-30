#pragma once

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
    Q_PROPERTY(QUrl trackUrl READ trackUrl WRITE setTrackUrl NOTIFY trackUrlChanged)
    Q_PROPERTY(Channels channels READ getChannels WRITE setChannels NOTIFY channelsChanged)
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
    void setTrackUrl(const QUrl& track);
    QUrl trackUrl() const {
        return m_trackUrl;
    }

    void setChannels(Channels channels);
    Channels getChannels() const;
  private slots:
    void slotWaveformUpdated();
    void slotOverviewChanged(TrackId trackId);

  signals:
    void trackChanged();
    void trackUrlChanged();
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
    QPixmap renderWaveformToPixmap(ConstWaveformPointer pWaveform, int completion) const;
    void invalidatePixmapCacheForCurrent();

    QPointer<QmlTrackProxy> m_pTrack;
    QUrl m_trackUrl;
    Channels m_channels;
    Renderer m_renderer;
    QColor m_colorHigh;
    QColor m_colorMid;
    QColor m_colorLow;

    ConstWaveformPointer m_waveformSummary;
    TrackId m_trackId;

    static QString pixmapCacheKey(TrackId trackId);
};

} // namespace qml
} // namespace mixxx
