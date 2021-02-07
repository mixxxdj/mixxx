#pragma once

#include <QPainter>
#include <QTime>
#include <QVector>
#include <QtDebug>

#include "track/track_decl.h"
#include "util/class.h"
#include "util/performancetimer.h"
#include "waveform/renderers/waveformmark.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "waveform/renderers/waveformsignalcolors.h"

//#define WAVEFORMWIDGETRENDERER_DEBUG

class ControlProxy;
class VisualPlayPosition;
class VSyncThread;

class WaveformWidgetRenderer {
  public:
    static const double s_waveformMinZoom;
    static const double s_waveformMaxZoom;
    static const double s_waveformDefaultZoom;
    static const double s_defaultPlayMarkerPosition;

  public:
    explicit WaveformWidgetRenderer(const QString& group);
    virtual ~WaveformWidgetRenderer();

    bool init();
    virtual bool onInit() {return true;}

    void setup(const QDomNode& node, const SkinContext& context);
    void onPreRender(VSyncThread* vsyncThread);
    void draw(QPainter* painter, QPaintEvent* event);

    const QString& getGroup() const {
        return m_group;
    }
    const TrackPointer getTrackInfo() const {
        return m_pTrack;
    }
    /// Get cue mark at a point on the waveform widget.
    WaveformMarkPointer getCueMarkAtPoint(QPoint point) const;

    double getFirstDisplayedPosition() const {
        return m_firstDisplayedPosition;
    }
    double getLastDisplayedPosition() const {
        return m_lastDisplayedPosition;
    }

    void setZoom(double zoom);

    void setDisplayBeatGrid(bool set);
    void setDisplayBeatGridAlpha(int alpha);

    double getVisualSamplePerPixel() const {
        return m_visualSamplePerPixel;
    }
    double getAudioSamplePerPixel() const {
        return m_audioSamplePerPixel;
    }

    // those function replace at its best sample position to an admissible
    // sample position according to the current visual resampling
    // this make mark and signal deterministic
    void regulateVisualSample(int& sampleIndex) const;

    // this "regulate" against visual sampling to make the position in widget
    // stable and deterministic
    // Transform sample index to pixel in track.
    inline double transformSamplePositionInRendererWorld(double samplePosition) const {
        const double relativePosition = samplePosition / m_trackSamples;
        return transformPositionInRendererWorld(relativePosition);
    }
    // Transform position (percentage of track) to pixel in track.
    inline double transformPositionInRendererWorld(double position) const {
        return m_trackPixelCount * (position - m_firstDisplayedPosition);
    }

    double getPlayPos() const {
        return m_playPos;
    }
    int getPlayPosVSample() const {
        return m_playPosVSample;
    }
    int getTotalVSample() const {
        return m_totalVSamples;
    }
    double getZoomFactor() const {
        return m_zoomFactor;
    }
    double getGain() const {
        return m_gain;
    }
    int getTrackSamples() const {
        return m_trackSamples;
    }

    int getBeatGridAlpha() const {
        return m_alphaBeatGrid;
    }

    void resize(int width, int height, float devicePixelRatio);
    int getHeight() const {
        return m_height;
    }
    int getWidth() const {
        return m_width;
    }
    float getDevicePixelRatio() const {
        return m_devicePixelRatio;
    }
    int getLength() const {
        return m_orientation == Qt::Horizontal ? m_width : m_height;
    }
    int getBreadth() const {
        return m_orientation == Qt::Horizontal ? m_height : m_width;
    }
    Qt::Orientation getOrientation() const {
        return m_orientation;
    }
    const WaveformSignalColors* getWaveformSignalColors() const {
        return &m_colors;
    }
    int getDimBrightThreshold() {
        return m_dimBrightThreshold;
    }

    template< class T_Renderer>
    inline T_Renderer* addRenderer() {
        T_Renderer* renderer = new T_Renderer(this);
        m_rendererStack.push_back(renderer);
        return renderer;
    }

    void setTrack(TrackPointer track);
    void setMarkPositions(const QMap<WaveformMarkPointer, int>& markPositions) {
        m_markPositions = markPositions;
    }

    double getPlayMarkerPosition() {
        return m_playMarkerPosition;
    }

    void setPlayMarkerPosition(double newPos) {
        VERIFY_OR_DEBUG_ASSERT(newPos >= 0.0 && newPos <= 1.0) {
            newPos = math_clamp(newPos, 0.0, 1.0);
        }
        m_playMarkerPosition = newPos;
    }

  protected:
    const QString m_group;
    TrackPointer m_pTrack;
    QList<WaveformRendererAbstract*> m_rendererStack;
    Qt::Orientation m_orientation;
    int m_dimBrightThreshold;
    int m_height;
    int m_width;
    float m_devicePixelRatio;
    WaveformSignalColors m_colors;

    double m_firstDisplayedPosition;
    double m_lastDisplayedPosition;
    double m_trackPixelCount;

    double m_zoomFactor;
    double m_visualSamplePerPixel;
    double m_audioSamplePerPixel;

    int m_alphaBeatGrid;

    //TODO: vRince create some class to manage control/value
    //ControlConnection
    QSharedPointer<VisualPlayPosition> m_visualPlayPosition;
    double m_playPos;
    int m_playPosVSample;
    int m_totalVSamples;
    ControlProxy* m_pRateRatioCO;
    double m_rateRatio;
    ControlProxy* m_pGainControlObject;
    double m_gain;
    ControlProxy* m_pTrackSamplesControlObject;
    int m_trackSamples;
    double m_scaleFactor;
    double m_playMarkerPosition;   // 0.0 - left, 0.5 - center, 1.0 - right

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    PerformanceTimer* m_timer;
    int m_lastFrameTime;
    int m_lastFramesTime[100];
    int m_lastSystemFrameTime;
    int m_lastSystemFramesTime[100];
    int currentFrame;
#endif

private:
    DISALLOW_COPY_AND_ASSIGN(WaveformWidgetRenderer);
    friend class WaveformWidgetFactory;
    QMap<WaveformMarkPointer, int> m_markPositions;
    // draw play position indicator triangles
    void drawPlayPosmarker(QPainter* painter);
    void drawTriangle(QPainter* painter,
            const QBrush& fillColor,
            QPointF p1,
            QPointF p2,
            QPointF p3);
};
