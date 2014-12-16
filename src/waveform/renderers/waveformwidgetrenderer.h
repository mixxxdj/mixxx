#ifndef WAVEFORMWIDGETRENDERER_H
#define WAVEFORMWIDGETRENDERER_H

#include <QPainter>
#include <QTime>
#include <QVector>
#include <QtDebug>

#include "trackinfoobject.h"
#include "util.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "waveform/renderers/waveformsignalcolors.h"

//#define WAVEFORMWIDGETRENDERER_DEBUG

class TrackInfoObject;
class ControlObjectThread;
class VisualPlayPosition;
class VSyncThread;

class WaveformWidgetRenderer {
  public:
    static const int s_waveformMinZoom;
    static const int s_waveformMaxZoom;

  public:
    explicit WaveformWidgetRenderer(const char* group);
    virtual ~WaveformWidgetRenderer();

    bool init();
    virtual bool onInit() {return true;}

    void setup(const QDomNode& node, const SkinContext& context);
    void onPreRender(VSyncThread* vsyncThread);
    void draw(QPainter* painter, QPaintEvent* event);

    inline const char* getGroup() const { return m_group;}
    const TrackPointer getTrackInfo() const { return m_pTrack;}

    double getFirstDisplayedPosition() const { return m_firstDisplayedPosition;}
    double getLastDisplayedPosition() const { return m_lastDisplayedPosition;}

    void setZoom(int zoom);

    double getVisualSamplePerPixel() const { return m_visualSamplePerPixel;};
    double getAudioSamplePerPixel() const { return m_audioSamplePerPixel;};

    //those function replace at its best sample position to an admissible
    //sample position according to the current visual resampling
    //this make mark and signal deterministic
    void regulateVisualSample(int& sampleIndex) const;

    //this "regulate" against visual sampling to make the position in widget
    //stable and deterministic
    // Transform sample index to pixel in track.
    inline double transformSampleIndexInRendererWorld(int sampleIndex) const {
        const double relativePosition = (double)sampleIndex / (double)m_trackSamples;
        return transformPositionInRendererWorld(relativePosition);
    }
    // Transform position (percentage of track) to pixel in track.
    inline double transformPositionInRendererWorld(double position) const {
        return m_trackPixelCount * (position - m_firstDisplayedPosition);
    }

    double getPlayPos() const { return m_playPos;}
    double getPlayPosVSample() const { return m_playPosVSample;}
    double getZoomFactor() const { return m_zoomFactor;}
    double getRateAdjust() const { return m_rateAdjust;}
    double getGain() const { return m_gain;}
    int getTrackSamples() const { return m_trackSamples;}

    void resize(int width, int height);
    int getHeight() const { return m_height;}
    int getWidth() const { return m_width;}
    const WaveformSignalColors* getWaveformSignalColors() const { return &m_colors; };

    template< class T_Renderer>
    inline T_Renderer* addRenderer() {
        T_Renderer* renderer = new T_Renderer(this);
        m_rendererStack.push_back(renderer);
        return renderer;
    }

    void setTrack(TrackPointer track);

  protected:
    const char* m_group;
    TrackPointer m_pTrack;
    QList<WaveformRendererAbstract*> m_rendererStack;
    int m_height;
    int m_width;
    WaveformSignalColors m_colors;

    double m_firstDisplayedPosition;
    double m_lastDisplayedPosition;
    double m_trackPixelCount;

    double m_zoomFactor;
    double m_rateAdjust;
    double m_visualSamplePerPixel;
    double m_audioSamplePerPixel;

    //TODO: vRince create some class to manage control/value
    //ControlConnection
    QSharedPointer<VisualPlayPosition> m_visualPlayPosition;
    double m_playPos;
    int m_playPosVSample;
    ControlObjectThread* m_pRateControlObject;
    double m_rate;
    ControlObjectThread* m_pRateRangeControlObject;
    double m_rateRange;
    ControlObjectThread* m_pRateDirControlObject;
    double m_rateDir;
    ControlObjectThread* m_pGainControlObject;
    double m_gain;
    ControlObjectThread* m_pTrackSamplesControlObject;
    int m_trackSamples;

#ifdef WAVEFORMWIDGETRENDERER_DEBUG
    QTime* m_timer;
    int m_lastFrameTime;
    int m_lastFramesTime[100];
    int m_lastSystemFrameTime;
    int m_lastSystemFramesTime[100];
    int currentFrame;
#endif

private:
    DISALLOW_COPY_AND_ASSIGN(WaveformWidgetRenderer);
    friend class WaveformWidgetFactory;
};

#endif // WAVEFORMWIDGETRENDERER_H
