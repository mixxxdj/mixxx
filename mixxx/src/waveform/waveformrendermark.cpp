
#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>

#include "waveformrendermark.h"

#include "waveformrenderer.h"
#include "configobject.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "wskincolor.h"
#include "wwidget.h"
#include "trackinfoobject.h"

WaveformRenderMark::WaveformRenderMark(const char *group, ConfigKey key, WaveformRenderer *parent) {

    m_pParent = parent;
    m_key = key;

    m_iMarkPoint = -1;
    m_iSampleRate = -1;
    m_dSamplesPerDownsample = -1;
    m_iNumSamples = 0;
    
    m_pMarkPoint = new ControlObjectThreadMain(ControlObject::getControl(key));
    connect(m_pMarkPoint, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateMarkPoint(double)));

    m_pTrackSamples = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group,"track_samples")));
    slotUpdateTrackSamples(m_pTrackSamples->get());
    connect(m_pTrackSamples, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateTrackSamples(double)));
}

void WaveformRenderMark::slotUpdateMarkPoint(double v) {
    qDebug() << "WaveformRenderMark :: MarkPoint = " << v;
    m_iMarkPoint = (int)v;
}

void WaveformRenderMark::slotUpdateTrackSamples(double samples) {
    qDebug() << "WaveformRenderMark :: samples = " << int(samples);
    m_iNumSamples = (int)samples;
}

void WaveformRenderMark::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
}

void WaveformRenderMark::newTrack(TrackInfoObject* pTrack) {
    m_pTrack = pTrack;
    m_iMarkPoint = -1;
    m_iNumSamples = 0;

    // calculate beat info for this track:

    int sampleRate = pTrack->getSampleRate();

    // f = z * m * n
    double m = m_pParent->getSubpixelsPerPixel();
    double f = sampleRate;
    double z = m_pParent->getPixelsPerSecond();
    double n = f / (m*z);

    m_iSampleRate = sampleRate;

    m_dSamplesPerDownsample = n;

}

void WaveformRenderMark::setup(QDomNode node) {

    markColor.setNamedColor(WWidget::selectNodeQString(node, "CueColor"));
    markColor = WSkinColor::getCorrectColor(markColor);

}


void WaveformRenderMark::draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double dPlayPos, double rateAdjust) {

    if(m_iSampleRate == -1 || m_iSampleRate == 0 || m_iNumSamples == 0)
        return;

    // necessary?
    if(buffer == NULL)
        return;

    double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel()*(1.0+rateAdjust);

    pPainter->save();
    pPainter->scale(1.0/subpixelsPerPixel,1.0);
    pPainter->setPen(markColor);
    
    double subpixelWidth = m_iWidth * subpixelsPerPixel;
    double subpixelHalfWidth = subpixelWidth / 2.0;
    double halfh = m_iHeight/2;
    
    if(m_iMarkPoint != -1) {
        double markPointMono = m_iMarkPoint >> 1;
        double curPos = dPlayPos * (m_iNumSamples/2);
        double i = (markPointMono - curPos)/m_dSamplesPerDownsample;
        
        if(abs(i) < subpixelHalfWidth) {
            double x = (i+subpixelHalfWidth);
            pPainter->drawLine(QLineF(x, halfh, x, -halfh));
        }
    }

    pPainter->restore();
}
