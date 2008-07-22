
#ifndef WAVEFORMRENDERER_H
#define WAVEFORMRENDERER_H

#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QVector>

#include "defs.h"

class TrackInfoObject;
class ControlObjectThreadMain;
class QDomNode;
class WaveformRenderBeat;
class WaveformRenderMark;
class ControlObject;

class WaveformRenderer : public QObject {
    Q_OBJECT
public:
    WaveformRenderer(const char* group);
    ~WaveformRenderer();

    void resize(int w, int h);
    void draw(QPainter* pPainter, QPaintEvent *pEvent);
    void drawSignalLines(QPainter*, double playpos);
    void drawSignalPixmap(QPainter* p);
    void newTrack(TrackInfoObject *pTrack);
    void setup(QDomNode node);
    void precomputePixmap();
    int getSubpixelsPerPixel();
    int getPixelsPerSecond();
public slots:
    void slotUpdatePlayPos(double playpos);

private:
    void generateBackgroundPixmap();
    void setupControlObjects();
    bool fetchWaveformFromTrack();
    int m_iWidth, m_iHeight;
    QColor bgColor, signalColor, colorMarker, colorBeat, colorCue;
    int m_iNumSamples;

    int m_iPlayPosTime, m_iPlayPosTimeOld;
    double m_dPlayPos, m_dPlayPosOld;

    QVector<float> *m_pSampleBuffer;
    QVector<QLineF> m_lines;
    QPixmap m_backgroundPixmap;
    bool m_bRepaintBackground;
    QPixmap *m_pPixmap;
    QImage m_pImage;

    ControlObjectThreadMain *m_pPlayPos;

    ControlObject *m_pCOVisualResample;

    WaveformRenderBeat *m_pRenderBeat;
    WaveformRenderMark *m_pRenderCue;

    const int m_iSubpixelsPerPixel;
    const int m_iPixelsPerSecond;
    TrackInfoObject *m_pTrack;
    
};

#endif
