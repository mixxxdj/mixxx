
#ifndef WAVEFORMRENDERMARK_H
#define WAVEFORMRENDERMARK_H

#include <QObject>
#include <QColor>
#include <QPixmap>
#include <QVector>

class QDomNode;
class QPainter;
class QPaintEvent;

#include "configobject.h"
#include "waveform/renderobject.h"

class ConfigKey;
class ControlObjectThreadMain;
class WaveformRenderer;

class WaveformRenderMark : public RenderObject {
    Q_OBJECT
public:
    explicit WaveformRenderMark(const char* pGroup,
                                WaveformRenderer *parent);
    void resize(int w, int h);
    void setup(QDomNode node);
    void draw(QPainter *pPainter, QPaintEvent *event,
              QVector<float> *buffer, double playPos, double rateAdjust);
    void newTrack(TrackPointer pTrack);

public slots:
    void slotUpdateMarkPoint(double mark);
    void slotUpdateTrackSamples(double samples);
private:
    void setupMarkPixmap();

    enum MarkAlign {
        TOP = 0,
        BOTTOM,
        CENTER
    };

    const char* m_pGroup;
    WaveformRenderer *m_pParent;
    ControlObjectThreadMain *m_pMarkPoint;
    ControlObjectThreadMain *m_pTrackSamples;
    TrackPointer m_pTrack;

    int m_iMarkPoint;
    int m_iWidth, m_iHeight;
    QColor m_markColor;
    QColor m_textColor;
    QString m_markText;
    QString m_markPixmapPath;
    MarkAlign m_markAlign;
    QPixmap m_markPixmap;
    double m_dSamplesPerDownsample;

    int m_iNumSamples;
    int m_iSampleRate;
};

#endif
