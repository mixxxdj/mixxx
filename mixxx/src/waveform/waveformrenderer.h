
#ifndef WAVEFORMRENDERER_H
#define WAVEFORMRENDERER_H

#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QVector>
#include <QTime>
#include <QThread>

#include "defs.h"

class TrackInfoObject;
class ControlObjectThreadMain;
class QDomNode;
class WaveformRenderBackground;
class WaveformRenderSignal;
class WaveformRenderSignalPixmap;
class WaveformRenderBeat;
class WaveformRenderMark;
class ControlObject;

class WaveformRenderer : public QThread {
    Q_OBJECT
public:
    WaveformRenderer(const char* group);
    ~WaveformRenderer();

    void resize(int w, int h);
    void draw(QPainter* pPainter, QPaintEvent *pEvent);
    void drawSignalPixmap(QPainter* p);
    void setup(QDomNode node);
    void precomputePixmap();
    int getSubpixelsPerPixel();
    int getPixelsPerSecond();
public slots:
    void slotNewTrack(TrackInfoObject *pTrack);
    void slotUpdateLatency(double latency);
    void slotUpdatePlayPos(double playpos);
    void slotUpdateRate(double rate);
    void slotUpdateRateRange(double rate);
    
protected:
    void run();

private:
    void setupControlObjects();
    bool fetchWaveformFromTrack();
    int m_iWidth, m_iHeight;
    QColor bgColor, signalColor, colorMarker, colorBeat, colorCue;
    int m_iNumSamples;

    int m_iPlayPosTime, m_iPlayPosTimeOld;
    QTime m_playPosTime, m_playPosTimeOld;
    double m_dPlayPos, m_dPlayPosOld, m_dRate, m_dRateRange;
    int m_iDupes;
    double m_dPlayPosAdjust;
    int m_iLatency;
    

    QVector<float> *m_pSampleBuffer;
    QPixmap *m_pPixmap;
    QImage m_pImage;

    ControlObjectThreadMain *m_pLatency;
    ControlObjectThreadMain *m_pPlayPos;
    ControlObjectThreadMain *m_pRate;
    ControlObjectThreadMain *m_pRateRange;
    ControlObjectThreadMain *m_pRateDir;
    
    ControlObject *m_pCOVisualResample;

    WaveformRenderBackground *m_pRenderBackground;
    WaveformRenderSignal *m_pRenderSignal;
    WaveformRenderSignalPixmap *m_pRenderSignalPixmap;
    WaveformRenderBeat *m_pRenderBeat;
    WaveformRenderMark *m_pRenderCue;
    WaveformRenderMark *m_pRenderLoopStart;
    WaveformRenderMark *m_pRenderLoopEnd;

    const int m_iSubpixelsPerPixel;
    const int m_iPixelsPerSecond;
    TrackInfoObject *m_pTrack;

    bool m_bQuit;
    
};

#endif
