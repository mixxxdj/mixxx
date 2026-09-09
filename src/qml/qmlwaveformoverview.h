#pragma once

#include <QPainter>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickPaintedItem>

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
    Q_PROPERTY(mixxx::qml::QmlWaveformOverview::Renderer renderer READ
                    renderer WRITE setRenderer NOTIFY rendererChanged)
    Q_PROPERTY(bool stereo READ stereo WRITE setStereo NOTIFY stereoChanged)
    Q_PROPERTY(bool normalized READ normalized WRITE setNormalized NOTIFY
                    normalizedChanged)
    Q_PROPERTY(bool minuteMarkers READ minuteMarkers WRITE setMinuteMarkers
                    NOTIFY minuteMarkersChanged)
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
        HSV = 3,
    };
    Q_ENUM(Renderer)

    QmlWaveformOverview(QQuickItem* parent = nullptr);
    ~QmlWaveformOverview() override = default;

    void paint(QPainter* painter) override;

    void setTrack(QmlTrackProxy* track);
    QmlTrackProxy* getTrack() const;

    void setChannels(Channels channels);
    Channels getChannels() const;
    Renderer renderer() const {
        return m_renderer;
    }
    void setRenderer(Renderer renderer);
    bool stereo() const {
        return m_stereo;
    }
    void setStereo(bool stereo);
    bool normalized() const {
        return m_normalized;
    }
    void setNormalized(bool normalized);
    bool minuteMarkers() const {
        return m_minuteMarkers;
    }
    void setMinuteMarkers(bool minuteMarkers);
  private slots:
    void slotWaveformUpdated();

  signals:
    void trackChanged();
    void channelsChanged(mixxx::qml::QmlWaveformOverview::Channels channels);
    void rendererChanged(mixxx::qml::QmlWaveformOverview::Renderer renderer);
    void stereoChanged();
    void normalizedChanged();
    void minuteMarkersChanged();
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
    void drawHsv(QPainter* pPainter,
            Channels channels,
            ConstWaveformPointer pWaveform,
            int completion) const;
    void drawMinuteMarkers(QPainter* pPainter, double duration) const;
    QColor getRgbPenColor(ConstWaveformPointer pWaveform, int completion) const;
    QmlTrackProxy* m_pTrack;
    Channels m_channels;
    Renderer m_renderer;
    bool m_stereo;
    bool m_normalized;
    bool m_minuteMarkers;
    QColor m_colorHigh;
    QColor m_colorMid;
    QColor m_colorLow;
};

} // namespace qml
} // namespace mixxx
