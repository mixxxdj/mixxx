#ifndef WAVEFORMWIDGETRENDERER_H
#define WAVEFORMWIDGETRENDERER_H

#include <QPainter>
#include <QTime>
#include <QVector>
#include <QtDebug>

#include "trackinfoobject.h"
#include "util.h"
#include "waveform/renderers/waveformrendererabstract.h"

class TrackInfoObject;
class ControlObjectThreadMain;

class WaveformWidgetRenderer {
  public:
    explicit WaveformWidgetRenderer(const char* group);
    virtual ~WaveformWidgetRenderer();

    void init();
    void setup(const QDomNode& node);
    void preRender();
    void draw(QPainter* painter, QPaintEvent* event);

    const char* getGroup() const { return m_group;}
    const TrackPointer getTrackInfo() const { return m_trackInfoObject;}

    double getFirstDisplayedPosition() const { return m_firstDisplayedPosition;}
    double getLastDisplayedPosition() const { return m_lastDisplayedPosition;}

    void setZoom(int zoom);
    void zoomIn();
    void zoomOut();

    virtual void updateVisualSamplingPerPixel();
    virtual void updateAudioSamplingPerPixel();

    double getVisualSamplePerPixel() const;
    double getAudioSamplePerPixel() const;

    //those function replace at its best sample position to an admissible
    //sample position according to the current visual resampling
    //this make mark and signal deterministic
    void regulateVisualSample(int& sampleIndex) const;
    void regulateAudioSample(int& sampleIndex) const;

    //this "regulate" against visual sampling to make the position in widget
    //stable and deterministic
    double transformSampleIndexInRendererWorld(int sampleIndex) const;
    double transformPositionInRendererWorld(double position) const;

    double getPlayPos() const { return m_playPos;}
    double getZoomFactor() const { return m_zoomFactor;}
    double getRateAdjust() const { return m_rateAdjust;}
    double getGain() const { return m_gain;}
    int getTrackSamples() const { return m_trackSamples;}

    void resize(int width, int height);
    int getHeight() const { return m_height;}
    int getWidth() const { return m_width;}

    template< class T_Renderer>
    inline T_Renderer* addRenderer() {
        T_Renderer* renderer = new T_Renderer(this);
        m_rendererStack.push_back(renderer);
        return renderer;
    }

    void setTrack(TrackPointer track);

  protected:
    const char* m_group;
    TrackPointer m_trackInfoObject;
    QVector<WaveformRendererAbstract*> m_rendererStack;
    int m_height;
    int m_width;

    double m_firstDisplayedPosition;
    double m_lastDisplayedPosition;
    double m_rendererTransformationOffset;
    double m_rendererTransformationGain;

    double m_rateAdjust;
    double m_zoomFactor;
    double m_visualSamplePerPixel;
    double m_audioSamplePerPixel;

    //TODO vRince create some class to manage control/value
    //ControlConnection
    ControlObject* m_playPosControlObject;
    double m_playPos;
    ControlObject* m_rateControlObject;
    double m_rate;
    ControlObject* m_rateRangeControlObject;
    double m_rateRange;
    ControlObject* m_rateDirControlObject;
    double m_rateDir;
    ControlObject* m_gainControlObject;
    double m_gain;
    ControlObject* m_trackSamplesControlObject;
    int m_trackSamples;

    //Debug
    QTime* m_timer;
    int m_lastFrameTime;
    int m_lastFramesTime[100];
    int m_lastSystemFrameTime;
    int m_lastSystemFramesTime[100];
    int currentFrame;

    WaveformWidgetRenderer();
  private:
    DISALLOW_COPY_AND_ASSIGN(WaveformWidgetRenderer);
    friend class WaveformWidgetFactory;
};

#endif // WAVEFORMWIDGETRENDERER_H
