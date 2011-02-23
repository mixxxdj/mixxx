
#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>

#include "waveformrenderbeat.h"
#include "waveformrenderer.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "trackinfoobject.h"

WaveformRenderBeat::WaveformRenderBeat(const char* group, WaveformRenderer *parent)
        : m_pParent(parent),
          m_pBpm(NULL),
          m_pBeatFirst(NULL),
          m_pTrackSamples(NULL),
          m_pTrack(),
          m_iWidth(0),
          m_iHeight(0),
          m_dBpm(-1),
          m_dBeatFirst(-1),
          m_dSamplesPerPixel(-1),
          m_dSamplesPerDownsample(-1),
          m_dBeatLength(-1),
          m_iNumSamples(0),
          m_iSampleRate(-1) {
    m_pBpm = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "file_bpm")));
    connect(m_pBpm, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateBpm(double)));

    //m_pBeatFirst = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "BeatFirst")));
    //connect(m_pBeatFirst, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateBeatFirst(double)));

    m_pTrackSamples = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group,"track_samples")));
    slotUpdateTrackSamples(m_pTrackSamples->get());
    connect(m_pTrackSamples, SIGNAL(valueChanged(double)),
            this, SLOT(slotUpdateTrackSamples(double)));
}

void WaveformRenderBeat::slotUpdateBpm(double v) {
    //qDebug() << "WaveformRenderBeat :: BPM = " << v;
    m_dBpm = v;
    m_dBeatLength = -1;
}

void WaveformRenderBeat::slotUpdateBeatFirst(double v) {
    //qDebug() << "WaveformRenderBeat :: beatFirst = " << v;
    m_dBeatFirst = v;
}

void WaveformRenderBeat::slotUpdateTrackSamples(double samples) {
    //qDebug() << "WaveformRenderBeat :: samples = " << int(samples);
    m_iNumSamples = (int)samples;
}

void WaveformRenderBeat::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
}

void WaveformRenderBeat::newTrack(TrackPointer pTrack) {
    m_pTrack = pTrack;
    m_dBpm = -1;
    m_dBeatFirst = -1;
    m_dBeatLength = -1;
    m_iNumSamples = 0;
    m_iSampleRate = 0;
    m_dSamplesPerDownsample = -1;
    m_dSamplesPerPixel = -1;

    if (!m_pTrack)
        return;

    // calculate beat info for this track:

    int sampleRate = pTrack->getSampleRate();

    // f = z * m * n
    double m = m_pParent->getSubpixelsPerPixel();
    double f = sampleRate;
    double z = m_pParent->getPixelsPerSecond();
    double n = f / (m*z);

    m_iSampleRate = sampleRate;

    m_dSamplesPerDownsample = n;
    m_dSamplesPerPixel = double(f)/z;

    //qDebug() << "WaveformRenderBeat sampleRate  " << sampleRate << " samplesPerPixel " << m_dSamplesPerPixel;

}

void WaveformRenderBeat::setup(QDomNode node) {

    colorMarks.setNamedColor(WWidget::selectNodeQString(node, "BeatColor"));
    colorMarks = WSkinColor::getCorrectColor(colorMarks);

    colorHighlight = Qt::black;
    QString highlight = WWidget::selectNodeQString(node, "BeatHighlightColor");
    if (highlight != "") {
        colorHighlight.setNamedColor(highlight);
    }
    colorHighlight = WSkinColor::getCorrectColor(colorHighlight);
}


void WaveformRenderBeat::draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double dPlayPos, double rateAdjust) {
    if(m_dBpm == -1 || m_dBpm == 0)
        return;

    slotUpdateTrackSamples(m_pTrackSamples->get());

    if(m_iSampleRate == -1 || m_iSampleRate == 0 || m_iNumSamples == 0)
        return;

    if(buffer == NULL)
        return;

    int iCurPos = (int)(dPlayPos * m_iNumSamples);

    if(iCurPos % 2 != 0)
        iCurPos--;

    // iCurPos is the current sample being processed the current pixel
    // p, with respect to iCurPos is in the screen if it is less than
    // halfw from iCurPos. A sample is a beat if it satisifes the following:

    // for b beats per minute, that means b/60 beats per seconds, or 60/b seconds per beat.
    // with a sample rate of f (generally 44khz),
    //   60f/b = samples per beat

    // Therefore, sample s is a beat if it satisfies  s % 60f/b == 0.
    // where s is a /mono/ sample

    // beat length in samples
    if(m_dBeatLength <= 0) {
        m_dBeatLength = 60.0 * m_iSampleRate / m_dBpm;
    }

    double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel()*(1.0+rateAdjust);
    const int oversample = (int)subpixelsPerPixel;

    pPainter->save();
    pPainter->scale(1.0/subpixelsPerPixel,1.0);
    pPainter->setPen(colorMarks);

    double subpixelWidth = m_iWidth * subpixelsPerPixel;
    double subpixelHalfWidth = subpixelWidth / 2.0;
    double halfw = m_iWidth/2;
    double halfh = m_iHeight/2;

    // NOTE: converting curpos from stereo samples to mono samples
    iCurPos = iCurPos >> 1;

    // basePos and endPos are in samples
    double basePos = iCurPos - m_dSamplesPerPixel*halfw*(1.0+rateAdjust);
    double endPos = basePos + m_iWidth*m_dSamplesPerPixel*(1.0+rateAdjust);

    // snap to the first beat
    double curPos = ceilf(basePos/m_dBeatLength)*m_dBeatLength;

    bool reset = false;
    for(;curPos <= endPos; curPos+=m_dBeatLength) {
        if(curPos < 0)
            continue;

        // i relative to the current play position in subpixels
        double i = (curPos - iCurPos)/m_dSamplesPerDownsample;

        // If i is less than 20 subpixels from center, highlight it.
        if(abs(i) < 20) {
            pPainter->setPen(colorHighlight);
            reset = true;
        }

        // Translate from -subpixelHalfWidth..subpixelHalfwidth to 0..subpixelWidth
        i += subpixelHalfWidth;

        pPainter->drawLine(QLineF(i,halfh,i,-halfh));

        if(reset) {
            pPainter->setPen(colorMarks);
            reset = false;
        }
    }

    pPainter->restore();

}
