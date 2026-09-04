#pragma once

#include <QImage>
#include <QMutex>
#include <QPainter>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QTimer>

#include "qmltrackproxy.h"
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
    Q_PROPERTY(bool normalized MEMBER m_normalized NOTIFY normalizedChanged)
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

  signals:
    void trackChanged();
    void channelsChanged(mixxx::qml::QmlWaveformOverview::Channels channels);
    void rendererChanged(mixxx::qml::QmlWaveformOverview::Renderer renderer);
    void colorHighChanged(const QColor& color);
    void colorMidChanged(const QColor& color);
    void colorLowChanged(const QColor& color);
    void normalizedChanged();

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
    bool drawNextWaveformPart();
    void invalidateWaveformCache();
    QmlTrackProxy* m_pTrack;
    Channels m_channels;
    Renderer m_renderer;
    QColor m_colorHigh;
    QColor m_colorMid;
    QColor m_colorLow;
    bool m_normalized{false};
    QTimer m_waveformProgressTimer;
    ConstWaveformPointer m_cachedWaveform;
    QImage m_waveformImage;
    int m_actualCompletion{0};
    float m_waveformPeak{1.0f};
    mutable QMutex m_waveformCacheMutex;
};

} // namespace qml
} // namespace mixxx
