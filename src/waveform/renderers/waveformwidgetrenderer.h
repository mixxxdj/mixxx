#pragma once

#include "track/track_decl.h"
#include "util/class.h"
#include "waveform/renderers/waveformmark.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "waveform/waveform.h"

//#define WAVEFORMWIDGETRENDERER_DEBUG

class ControlProxy;
class VisualPlayPosition;
class VSyncThread;
class QPainter;
class WaveformRendererAbstract;

namespace rendergraph {
class Context;
}

class WaveformWidgetRenderer {
  public:
    static const double s_waveformMinZoom;
    static const double s_waveformMaxZoom;
    static const double s_waveformDefaultZoom;
    static const double s_defaultPlayMarkerPosition;

    struct WaveformMarkOnScreen {
        WaveformMarkPointer m_pMark;
        int m_offsetOnScreen;
    };

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

    const TrackPointer& getTrackInfo() const {
        return m_pTrack;
    }

#ifdef __STEM__
    uint getSelectedStems() const {
        return m_selectedStems;
    }
#endif

    bool isSlipActive() const {
        return m_pos[::WaveformRendererAbstract::Play] != m_pos[::WaveformRendererAbstract::Slip];
    }

    ConstWaveformPointer getWaveform() const;

    /// Get cue mark at a point on the waveform widget.
    WaveformMarkPointer getCueMarkAtPoint(QPoint point) const;

    CuePointer getCuePointerFromIndex(int cueIndex) const;

    double getFirstDisplayedPosition(
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play) const {
        return m_firstDisplayedPosition[type];
    }
    double getLastDisplayedPosition(
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play) const {
        return m_lastDisplayedPosition[type];
    }

    double getTruePosSample(::WaveformRendererAbstract::PositionSource type =
                                    ::WaveformRendererAbstract::Play) const {
        return m_truePosSample[type];
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

    // this "regulate" against visual sampling to make the position in widget
    // stable and deterministic
    // Transform sample index to pixel in track.
    inline double transformSamplePositionInRendererWorld(double samplePosition,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play) const {
        if (std::abs(samplePosition - m_truePosSample[type]) < 1.f) {
            // When asked for the sample position that corresponds with the play
            // marker, return the play market pixel position. This avoids a rare
            // rounding issue where a marker at that sample position would be
            // 1 pixel off.
            return m_playMarkerPosition * getLength();
        }
        return (samplePosition - m_firstDisplayedPosition[type] * m_trackSamples) /
                2 / m_audioSamplePerPixel;
    }

    int getPlayPosVSample(::WaveformRendererAbstract::PositionSource type =
                                  ::WaveformRendererAbstract::Play) const {
        return m_posVSample[type];
    }
    int getTotalVSample() const {
        return m_totalVSamples;
    }
    double getZoomFactor() const {
        return m_zoomFactor;
    }
    double getGain(bool applyCompensation) const {
        // m_gain was always multiplied by 2.0, according to a comment:
        //
        //   "This gain adjustment compensates for an arbitrary /2 gain chop in
        //   EnginePregain. See the comment there."
        //
        // However, no comment there seems to explain this, and it resulted
        // in renderers that use the filtered.all data for the amplitude, to
        // be twice the expected value.
        // But without this compensation, renderers that use the combined
        // lo, mid, hi values became much lower than expected. By making this
        // optional we move the decision to each renderer whether to apply the
        // compensation or not, in order to have a more similar amplitude across
        // waveform renderers
        return applyCompensation ? m_gain * 2.f : m_gain;
    }
    double getTrackSamples() const {
        return m_trackSamples;
    }

    int getBeatGridAlpha() const {
        return m_alphaBeatGrid;
    }

    virtual void resizeRenderer(int width, int height, float devicePixelRatio);

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

    template<class T_Renderer, typename... Args>
    inline T_Renderer* addRenderer(Args&&... args) {
        T_Renderer* renderer = new T_Renderer(this, std::forward<Args>(args)...);
        m_rendererStack.push_back(renderer);
        return renderer;
    }

#ifdef __STEM__
    void selectStem(mixxx::StemChannelSelection stemMask);
#endif
    void setTrack(TrackPointer track);
    void setMarkPositions(const QList<WaveformMarkOnScreen>& markPositions) {
        m_markPositions = markPositions;
    }

    double getPlayMarkerPosition() const {
        return m_playMarkerPosition;
    }

    void setPlayMarkerPosition(double newPos) {
        VERIFY_OR_DEBUG_ASSERT(newPos >= 0.0 && newPos <= 1.0) {
            newPos = std::clamp(newPos, 0.0, 1.0);
        }
        m_playMarkerPosition = newPos;
    }

    void setPassThroughEnabled(bool enabled);

    bool shouldOnlyDrawBackground() const {
        return m_trackSamples <= 0.0 || m_pos[::WaveformRendererAbstract::Play] == -1;
    }

    void setContext(rendergraph::Context* pContext) {
        m_pContext = pContext;
    }

    rendergraph::Context* getContext() const {
        return m_pContext;
    }

  protected:
    const QString m_group;
    TrackPointer m_pTrack;
#ifdef __STEM__
    uint m_selectedStems;
#endif
    QList<WaveformRendererAbstract*> m_rendererStack;
    Qt::Orientation m_orientation;
    int m_dimBrightThreshold;
    int m_height;
    int m_width;
    float m_devicePixelRatio;
    WaveformSignalColors m_colors;
    QColor m_passthroughLabelColor;

    double m_firstDisplayedPosition[2];
    double m_lastDisplayedPosition[2];
    double m_trackPixelCount;

    double m_zoomFactor;
    double m_visualSamplePerPixel;
    double m_audioSamplePerPixel;

    int m_alphaBeatGrid;

    //TODO: vRince create some class to manage control/value
    //ControlConnection
    QSharedPointer<VisualPlayPosition> m_visualPlayPosition;
    int m_posVSample[2];
    int m_totalVSamples;
    std::unique_ptr<ControlProxy> m_pRateRatioCO;
    std::unique_ptr<ControlProxy> m_pGainControlObject;
    std::unique_ptr<ControlProxy> m_pTrackSamplesControlObject;
    double m_gain;
    double m_trackSamples;
    double m_scaleFactor;
    double m_playMarkerPosition;   // 0.0 - left, 0.5 - center, 1.0 - right

    // used by allshader waveformrenderers when used with rendergraph nodes
    rendergraph::Context* m_pContext;

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
    QList<WaveformMarkOnScreen> m_markPositions;
    // draw play position indicator triangles
    void drawPlayPosmarker(QPainter* painter);
    void drawTriangle(QPainter* painter,
            const QBrush& fillColor,
            QPointF p1,
            QPointF p2,
            QPointF p3);
    void drawPassthroughLabel(QPainter* painter);

    bool m_passthroughEnabled;
    double m_pos[2];
    double m_truePosSample[2];
};
