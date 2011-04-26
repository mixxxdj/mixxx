#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QObject>
#include <QVector>

#include "waveformrendererabstract.h"
#include "trackinfoobject.h"

#include <QDebug>

class QTime;
class QPainter;
class TrackInfoObject;
class WaveformRendererAbstract;
class ControlObjectThreadMain;

class WaveformWidgetRenderer : public QObject, public WaveformRendererAbstract
{
    Q_OBJECT

public:
    WaveformWidgetRenderer( const char* group);
    virtual ~WaveformWidgetRenderer();

    virtual void init();
    virtual void setup( const QDomNode& node);
    virtual void draw( QPainter* painter, QPaintEvent* event);

    const char* getGroup() const { return m_group;}
    const TrackPointer getTrackInfo() const { return m_trackInfoObject;}

    bool zoomIn();
    bool zoomOut();
    float getVisualSamplePerPixel();

    double getPlayPos() const { return m_playPos;}
    double getZoomFactor() const { m_zoomFactor;}
    double getRateAdjust() const { return m_rateAdjust;}
    double getGain() const { return m_gain;}
    double getLowFilterGain() const { return m_lowFilterGain;}
    double getMidFilterGain() const { return m_midFilterGain;}
    double getHighFilterGain() const { return m_highFilterGain;}
    bool isLowKilled() const { return m_lowKill;}
    bool isMidKilled() const { return m_midKill;}
    bool isHighKilled() const { return m_highKill;}

    void resize( int width, int height);
    int getHeight() const { return m_height;}
    int getWidth() const { return m_width;}

    template< class T_Renderer>
    inline void addRenderer() { m_rendererStack.push_back( new T_Renderer(this));}

public slots:
    //TODO vRince
    //Drop the TrackPointer since we are sure to unload the track before it's deleted
    //With a TrackPointer no forward declare
    void slotNewTrack( TrackPointer track);
    void slotUnloadTrack( TrackPointer track);

protected:
    const char* m_group;
    TrackPointer m_trackInfoObject;
    QVector<WaveformRendererAbstract*> m_rendererStack;
    int m_height;
    int m_width;

    double m_rateAdjust;
    double m_zoomFactor;

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
    //gain
    ControlObject* m_gainControlObject;
    double m_gain;
    ControlObject* m_lowFilterControlObject;
    double m_lowFilterGain;
    ControlObject* m_lowKillControlObject;
    bool m_lowKill;
    ControlObject* m_midFilterControlObject;
    double m_midFilterGain;
    ControlObject* m_midKillControlObject;
    bool m_midKill;
    ControlObject* m_highFilterControlObject;
    double m_highFilterGain;
    ControlObject* m_highKillControlObject;
    bool m_highKill;

    //Debug
    QTime* m_timer;
    int m_lastFrameTime;
    int m_lastFramesTime[100];
    int m_lastSystemFrameTime;
    int m_lastSystemFramesTime[100];
    int currentFrame;

};

#endif // WAVEFORMWIDGET_H
