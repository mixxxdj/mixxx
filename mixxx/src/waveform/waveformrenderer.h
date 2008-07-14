
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
    void setDesiredSecondsToDisplay(int seconds);
    int getDesiredSecondsToDisplay();

public slots:
    void slotUpdatePlayPos(double playpos);

private:
    void setupControlObjects();
    bool fetchWaveformFromTrack();
    int m_iWidth, m_iHeight;
    QColor bgColor, signalColor, colorMarker, colorBeat, colorCue;
    int m_iNumSamples, m_iMax, m_iMin;

    int m_iPlayPosTime, m_iPlayPosTimeOld;
    double m_dPlayPos, m_dPlayPosOld;

    QVector<float> *m_pSampleBuffer;
    QVector<QLineF> m_lines;
    QPixmap *m_pPixmap;
    QImage m_pImage;

    ControlObjectThreadMain *m_pPlayPos;

    ControlObject *m_pCOVisualResample;
    WaveformRenderBeat *m_pRenderBeat;
    int m_iDesiredSecondsToDisplay;
    TrackInfoObject *m_pTrack;
    
};

#endif
