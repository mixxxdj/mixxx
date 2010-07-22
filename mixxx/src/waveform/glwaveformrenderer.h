
#ifndef GLWAVEFORMRENDERER_H
#define GLWAVEFORMRENDERER_H

#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QVector>
#include <qgl.h>

#include "defs.h"
#include "trackinfoobject.h"

class ControlObjectThreadMain;
class QDomNode;
class WaveformRenderBeat;
class ControlObject;

class GLWaveformRenderer : public QObject {
    Q_OBJECT
public:
    GLWaveformRenderer(const char* group);
    ~GLWaveformRenderer();

    void resize(int w, int h);
    void draw(QPainter* pPainter, QPaintEvent *pEvent);
    void glDraw();
    void drawSignalLines();
    void drawSignalPixmap(QPainter* p);
    void newTrack(TrackPointer pTrack);
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
    double m_dPlayPos;
    QVector<float> *m_pSampleBuffer;

    ControlObjectThreadMain *m_pPlayPos;
    ControlObject *m_pCOVerticalScale;
    ControlObject *m_pCOVisualResample;

    int m_iDesiredSecondsToDisplay;
    TrackPointer m_pTrack;

    GLfloat *m_pInternalBuffer;
    int m_iInternalBufferSize;
};

#endif
