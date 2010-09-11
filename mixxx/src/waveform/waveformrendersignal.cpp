
#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>
#include <QLine>
#include <qgl.h>

#include "waveformrendersignal.h"
#include "waveformrenderer.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "trackinfoobject.h"

WaveformRenderSignal::WaveformRenderSignal(const char* group, WaveformRenderer *parent)
        : m_pParent(parent),
          m_iWidth(0),
          m_iHeight(0),
          m_lines(0),
          m_pTrack(NULL),
          signalColor(255,255,255) {
}

void WaveformRenderSignal::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
}

void WaveformRenderSignal::newTrack(TrackPointer pTrack) {
    m_pTrack = pTrack;
}

void WaveformRenderSignal::setup(QDomNode node) {
    signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    signalColor = WSkinColor::getCorrectColor(signalColor);
}

void WaveformRenderSignal::draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double dPlayPos, double rateAdjust) {
    if(buffer == NULL)
        return;

    float* baseBuffer = buffer->data();

    int numBufferSamples = buffer->size();
    int iCurPos = 0;
    iCurPos = (int)(dPlayPos*numBufferSamples);

    if((iCurPos % 2) != 0)
        iCurPos--;

    pPainter->save();

    pPainter->setPen(signalColor);

    double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel() * (1.0 + rateAdjust);

    int subpixelWidth = int(m_iWidth * subpixelsPerPixel);

    // If the array is not large enough, expand it.
    // Amortize the cost of this by requesting a factor of 2 more.
    if(m_lines.size() < subpixelWidth) {
        m_lines.resize(2*subpixelWidth);
    }

    int halfw = subpixelWidth/2;
    for(int i=0;i<subpixelWidth;i++) {
        // Start at curPos minus half the waveform viewer
        int thisIndex = iCurPos+2*(i-halfw);
        if(thisIndex >= 0 && (thisIndex+1) < numBufferSamples) {
            float sampl = baseBuffer[thisIndex];
            float sampr = baseBuffer[thisIndex+1];
            const qreal xPos = i/subpixelsPerPixel;
            m_lines[i].setLine(xPos, -sampr*0.40*m_iHeight,
                               xPos, sampl*0.40*m_iHeight);
        } else {
            m_lines[i].setLine(0,0,0,0);
        }
    }

    // Only draw lines that we have provided
    pPainter->drawLines(m_lines.data(), subpixelWidth);

    pPainter->restore();
}
