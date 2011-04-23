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

    //Debug
    QTime* m_timer;
    int m_lastFrameTime;
    int m_lastFramesTime[100];
    int m_lastSystemFrameTime;
    int m_lastSystemFramesTime[100];
    int currentFrame;

};

#endif // WAVEFORMWIDGET_H
