
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
#include "wskincolor.h"
#include "wwidget.h"
#include "trackinfoobject.h"
#include "soundsourceproxy.h"

WaveformRenderBeat::WaveformRenderBeat(const char* group, WaveformRenderer *parent) {

    m_pParent = parent;

    m_iCuePoint = -1;
    m_dBpm = -1;
    m_dBeatFirst = -1;
    m_iSampleRate = -1;
    
    m_iSamplesPerPixel = -1;
    m_iNumSamples = 0;
    
    m_pCuePoint = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "cue_point")));
    connect(m_pCuePoint, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateCuePoint(double)));

    m_pBpm = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "file_bpm")));
    connect(m_pBpm, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateBpm(double)));

    //m_pBeatFirst = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "BeatFirst")));
    //connect(m_pBeatFirst, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateBeatFirst(double)));

    m_pTrackSamples = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group,"track_samples")));
    slotUpdateTrackSamples(m_pTrackSamples->get());
    connect(m_pTrackSamples, SIGNAL(valueChanged(double)), this, SLOT(slotUpdateTrackSamples(double)));
}

void WaveformRenderBeat::slotUpdateCuePoint(double v) {
    qDebug() << "WaveformRenderBeat :: CuePoint = " << v;
    m_iCuePoint = (int)v;
}

void WaveformRenderBeat::slotUpdateBpm(double v) {
    qDebug() << "WaveformRenderBeat :: BPM = " << v;
    m_dBpm = v;
}

void WaveformRenderBeat::slotUpdateBeatFirst(double v) {
    qDebug() << "WaveformRenderBeat :: beatFirst = " << v;
    m_dBeatFirst = v;
}

void WaveformRenderBeat::slotUpdateTrackSamples(double samples) {
    qDebug() << "WaveformRenderBeat :: samples = " << samples;
    m_iNumSamples = (int)samples;
}

void WaveformRenderBeat::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
}

void WaveformRenderBeat::newTrack(TrackInfoObject* pTrack) {
    m_pTrack = pTrack;
    m_iCuePoint = -1;
    m_dBpm = -1;
    m_dBeatFirst = -1;
    m_iNumSamples = 0;

    // calculate beat info for this track:
    
    int desiredSecondsToDisplay = m_pParent->getDesiredSecondsToDisplay();
    int sampleRate = pTrack->getSampleRate();

    // true number of samples displayed on the screen (i.e. mono signal)
    int screenSamples = desiredSecondsToDisplay * sampleRate;

    int samplesPerPixel = screenSamples / m_iWidth;

    m_iSampleRate = sampleRate;
    m_iSamplesPerPixel = samplesPerPixel;

}

void WaveformRenderBeat::setup(QDomNode node) {

    colorMarks.setNamedColor(WWidget::selectNodeQString(node, "BeatColor"));
    colorMarks = WSkinColor::getCorrectColor(colorMarks);

    colorCue.setNamedColor(WWidget::selectNodeQString(node, "CueColor"));
    colorCue = WSkinColor::getCorrectColor(colorCue);

}


void WaveformRenderBeat::draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double dPlayPos) {

    if(m_dBpm == -1)
        return;

    if(m_iSampleRate == -1)
        return;

    if(m_iNumSamples == 0) {
        // This is a guard against us getting stuck without this number.
        slotUpdateTrackSamples(m_pTrackSamples->get());
        return;
    }

    if(buffer == NULL)
        return;
    
    int iCurPos = (int)(dPlayPos * m_iNumSamples);
    
    if(iCurPos % 2 != 0)
        iCurPos--;

    int halfw = m_iWidth/2;
    int halfh = m_iHeight/2;

    // iCurPos is the current sample being processed the current pixel
    // p, with respect to iCurPos is in the screen if it is less than
    // halfw from iCurPos. A sample is a beat if it satisifes the following:

    // for b beats per minute, that means b/60 beats per seconds, or 60/b seconds per beat.
    // with a sample rate of f (generally 44khz),
    //   60f/b = samples per beat

    // Therefore, sample s is a beat if it satisfies  s % 60f/b == 0.
    // where s is a /mono/ sample

    double foo_factor = 60 * m_iSampleRate / m_dBpm;

    // NOTE: converting curpos from stereo samples to mono samples
    iCurPos = iCurPos >> 1;
    
    pPainter->setPen(colorMarks);
    int basePos = iCurPos - m_iSamplesPerPixel*halfw;
    double startPos = ceilf(basePos/foo_factor)*foo_factor;
    double endPos = basePos + m_iWidth*m_iSamplesPerPixel;

    for(;startPos <= endPos; startPos+=foo_factor) {
        if(startPos < 0)
            continue;
        int i = int((startPos - double(iCurPos))/m_iSamplesPerPixel)+halfw;
        pPainter->drawLine(QLine(i,halfh,i,-halfh));
    }
    
    /*
    // search is slow! replace this
    for(int i=0; i<m_iWidth;i++) {
        int thisPos = (i-halfw)*m_iSamplesPerPixel + iCurPos;

        if(thisPos < 0)
            continue;
        
        if(thisPos % int(foo_factor) < m_iSamplesPerPixel) {
            // a beat!
            pPainter->drawLine(QLine(i,halfh,i,-halfh));
        }

        }*/

    if(m_iCuePoint != -1) {
        int cuePointMono = m_iCuePoint >> 1;
        int i = (cuePointMono - iCurPos)/m_iSamplesPerPixel;
        
        if(abs(i) < halfw) {
            pPainter->setPen(colorCue);
            pPainter->drawLine(QLine(i+halfw, halfh, i+halfw, -halfh));
        }
    }
    
    
}
