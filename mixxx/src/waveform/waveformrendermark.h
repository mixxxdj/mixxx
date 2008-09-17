
#ifndef WAVEFORMRENDERMARK_H
#define WAVEFORMRENDERMARK_H

#include <QObject>
#include <QColor>
#include <QVector>

class QDomNode;
class QPainter;
class QPaintEvent;

#include "configobject.h"


class ConfigKey;
class ControlObjectThreadMain;
class WaveformRenderer;
class TrackInfoObject;

class WaveformRenderMark : public QObject {
    Q_OBJECT
public:
    void resize(int w, int h);
    void setup(QDomNode node);
    WaveformRenderMark(const char *group, ConfigKey key, WaveformRenderer *parent);
    void draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double playPos, double rateAdjust);
    void newTrack(TrackInfoObject *pTrack);

public slots:
    void slotUpdateMarkPoint(double mark);
    void slotUpdateTrackSamples(double samples);
private:
    WaveformRenderer *m_pParent;
    ControlObjectThreadMain *m_pMarkPoint;
    ControlObjectThreadMain *m_pTrackSamples;
    TrackInfoObject *m_pTrack;

    int m_iMarkPoint;
    int m_iWidth, m_iHeight;
    QColor markColor;
    double m_dSamplesPerDownsample;

    ConfigKey m_key;

    int m_iNumSamples;
    int m_iSampleRate;
};

#endif
