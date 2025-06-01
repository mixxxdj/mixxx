#pragma once

#include <qtmetamacros.h>

#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGNode>
#include <QSGSimpleRectNode>
#include <chrono>

#include "qml/qmlplayerproxy.h"
#include "qml/qmlwaveformrenderer.h"
#include "track/track.h"
#include "util/performancetimer.h"
#include "waveform/isynctimeprovider.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/widgets/waveformwidgettype.h"

class WaveformRendererAbstract;

namespace allshader {
class WaveformWidget;
class WaveformRenderMark;
class WaveformRenderMarkRange;
} // namespace allshader
namespace rendergraph {
class Node;
class OpacityNode;
class TreeNode;
} // namespace rendergraph

namespace mixxx {
namespace qml {

class QmlPlayerProxy;
class QmlTrackProxy;

class QmlWaveformDisplay : public QQuickItem, VSyncTimeProvider, public WaveformWidgetRenderer {
    Q_OBJECT
    Q_FLAGS(Options)
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QmlPlayerProxy* player READ getPlayer WRITE setPlayer
                    NOTIFY playerChanged)
    Q_PROPERTY(QString group READ getGroup WRITE setGroup NOTIFY groupChanged)
    Q_PROPERTY(QmlTrackProxy* track READ getTrack WRITE setStaticTrack NOTIFY trackChanged)
    Q_PROPERTY(double position READ getPosition WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QQmlListProperty<QmlWaveformRendererFactory> renderers READ renderers)
    Q_PROPERTY(double zoom READ getZoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QColor backgroundColor READ getBackgroundColor WRITE
                    setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(WaveformRendererSignalBaseOptions options READ
                    options WRITE setOptions NOTIFY optionsChanged)
    Q_CLASSINFO("DefaultProperty", "renderers")
    QML_NAMED_ELEMENT(WaveformDisplay)

  public:
    enum class Type {
        Simple = WaveformWidgetType::Simple,
        Filtered = WaveformWidgetType::Filtered,
        HSV = WaveformWidgetType::HSV,
        VSyncTest = WaveformWidgetType::VSyncTest,
        RGB = WaveformWidgetType::RGB,
        Stacked = WaveformWidgetType::Stacked,
    };
    Q_ENUM(Type);
    enum class Option : int {
        None = static_cast<int>(
                allshader::WaveformRendererSignalBase::Option::None),
        SplitStereoSignal = static_cast<int>(allshader::
                        WaveformRendererSignalBase::Option::SplitStereoSignal),
        HighDetail = static_cast<int>(
                allshader::WaveformRendererSignalBase::Option::HighDetail),
    };
    Q_DECLARE_FLAGS(Options, Option);

    QmlWaveformDisplay(QQuickItem* parent = nullptr);
    ~QmlWaveformDisplay() override;

    void setPlayer(QmlPlayerProxy* player);
    QmlPlayerProxy* getPlayer() const;

    QColor getBackgroundColor() const {
        return m_backgroundColor;
    }
    void setBackgroundColor(QColor color) {
        m_backgroundColor = color;
        m_dirtyFlag.setFlag(DirtyFlag::Background, true);
        emit backgroundColorChanged();
    }

    void setGroup(const QString& group) override;
    void setPosition(double position);
    void setStaticTrack(QmlTrackProxy* track);
    QmlTrackProxy* getTrack() const {
        return m_pTrack;
    }
    double getPosition() const;
    void setZoom(double zoom) {
        WaveformWidgetRenderer::setZoom(zoom);
        emit zoomChanged();
    }

    std::chrono::microseconds fromTimerToNextSync(const PerformanceTimer& timer) override;
    std::chrono::microseconds getSyncInterval() const override {
        return m_syncInterval;
    }

    void componentComplete() override;

    QQmlListProperty<QmlWaveformRendererFactory> renderers();
    static void renderers_append(
            QQmlListProperty<QmlWaveformRendererFactory>* property,
            QmlWaveformRendererFactory* value);
    static qsizetype renderers_count(QQmlListProperty<QmlWaveformRendererFactory>* property);
    static QmlWaveformRendererFactory* renderers_at(
            QQmlListProperty<QmlWaveformRendererFactory>* property, qsizetype index);
    static void renderers_clear(QQmlListProperty<QmlWaveformRendererFactory>* property);

    WaveformRendererSignalBaseOptions options() const {
        return m_options;
    }
    void setOptions(WaveformRendererSignalBaseOptions options);

  protected:
    QSGNode* updatePaintNode(QSGNode* old, QQuickItem::UpdatePaintNodeData*) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
  private slots:
    void slotTrackLoaded(TrackPointer pLoadedTrack);
    void slotTrackLoading(TrackPointer pNewTrack, TrackPointer pOldTrack);
    void slotTrackUnloaded();
    void slotWaveformUpdated();

    void slotFrameSwapped();
    void slotWindowChanged(QQuickWindow* window);
  signals:
    void playerChanged();
    void zoomChanged();
    void groupChanged(const QString& group);
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    void trackChanged(QmlTrackProxy* track);
#else
    void trackChanged(mixxx::qml::QmlTrackProxy* track);
#endif
    void positionChanged(double);
    void backgroundColorChanged();
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    void optionsChanged(WaveformRendererSignalBaseOptions);
#else
    void optionsChanged(mixxx::qml::WaveformRendererSignalBaseOptions);
#endif

  private:
    void setCurrentTrack(TrackPointer pTrack);

    // Properties
    QPointer<QmlPlayerProxy> m_pPlayer;
    QColor m_backgroundColor{QColor(0, 0, 0, 255)};

    PerformanceTimer m_timer;
    QmlTrackProxy* m_pTrack;
    QSharedPointer<VisualPlayPosition> m_visualPlayPosition;

    std::chrono::milliseconds m_syncInterval;
    enum class DirtyFlag : int {
        None = 0x0,
        Geometry = 0x1,
        Window = 0x2,
        Background = 0x4,
    };
    Q_DECLARE_FLAGS(DirtyFlags, DirtyFlag)

    DirtyFlags m_dirtyFlag{DirtyFlag::None};
    QList<QmlWaveformRendererFactory*> m_waveformRenderers;
    WaveformRendererSignalBaseOptions m_options{
            allshader::WaveformRendererSignalBase::Option::None};
};

} // namespace qml
} // namespace mixxx
