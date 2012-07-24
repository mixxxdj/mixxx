
#ifndef WAVEFORMRENDERBEAT_H
#define WAVEFORMRENDERBEAT_H

#include <QObject>
#include <QColor>
#include <QVector>

#include "renderobject.h"

class QDomNode;
class QPainter;
class QPaintEvent;

class ControlObjectThreadMain;
class WaveformRenderer;
class SoundSourceProxy;

class WaveformRenderBeat : public RenderObject {
    Q_OBJECT
  public:
    WaveformRenderBeat(const char *group, WaveformRenderer *parent);
    virtual ~WaveformRenderBeat();

    void resize(int w, int h);
    void setup(QDomNode node);
    void draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer,
              double playPos, double rateAdjust);
    void newTrack(TrackPointer pTrack);

  private slots:
    void slotUpdateTrackSamples(double samples);
    void slotUpdateBeatActive(double beatActive);
    void slotUpdateTrackSampleRate(double sampleRate);

  private:
    WaveformRenderer *m_pParent;
    ControlObjectThreadMain* m_pTrackSamples;
    ControlObjectThreadMain *m_pTrackSampleRate;
    ControlObjectThreadMain* m_pBeatActive;
    TrackPointer m_pTrack;
    int m_iWidth, m_iHeight;
    QColor colorMarks;
    QColor colorHighlight;
    double m_dSamplesPerPixel;
    double m_dSamplesPerDownsample;
    int m_iNumSamples;
    int m_iSampleRate;
    bool m_bBeatActive;
    BeatList m_beatList;
};

#endif
