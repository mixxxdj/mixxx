// waveformrendermarkrange.h
// Created 11/14/2009 by RJ Ryan (rryan@mit.edu)

#ifndef WAVEFORMRENDERMARKRANGE_H
#define WAVEFORMRENDERMARKRANGE_H

#include <QObject>
#include <QColor>
#include <QVector>

class QDomNode;
class QPainter;
class QPaintEvent;

#include "configobject.h"
#include "waveform/renderobject.h"

class ConfigKey;
class ControlObjectThreadMain;
class WaveformRenderer;

class WaveformRenderMarkRange : public RenderObject {
    Q_OBJECT
  public:
    WaveformRenderMarkRange(const char* pGroup, WaveformRenderer *parent);
    virtual ~WaveformRenderMarkRange();

    void resize(int w, int h);
    void setup(QDomNode node);
    void draw(QPainter *pPainter, QPaintEvent *event,
              QVector<float> *buffer, double playPos, double rateAdjust);
    void newTrack(TrackPointer pTrack);

  public slots:
    void slotUpdateMarkStartPoint(double mark);
    void slotUpdateMarkEndPoint(double mark);
    void slotUpdateMarkEnabled(double mark);
    void slotUpdateTrackSamples(double samples);
    void slotUpdateTrackSampleRate(double samples);

  private:
    const char* m_pGroup;
    WaveformRenderer *m_pParent;

    ControlObjectThreadMain *m_pMarkStartPoint;
    ControlObjectThreadMain *m_pMarkEndPoint;
    ControlObjectThreadMain *m_pMarkEnabled;
    ControlObjectThreadMain *m_pTrackSamples;
    ControlObjectThreadMain *m_pTrackSampleRate;

    bool m_bMarkEnabled;
    int m_iMarkStartPoint, m_iMarkEndPoint;
    int m_iWidth, m_iHeight;
    QColor m_markColor;
    QColor m_markDisabledColor;

    double m_dSamplesPerDownsample;
    int m_iNumSamples;
    int m_iSampleRate;
};

#endif
