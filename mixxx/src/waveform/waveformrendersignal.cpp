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
    m_fGain(1),
    m_lines(0),
    m_pTrack(NULL),
    signalColor(255,255,255) {
    m_pGain = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(group, "total_gain")));
    if (m_pGain != NULL) {
        connect(m_pGain, SIGNAL(valueChanged(double)),
                this, SLOT(slotUpdateGain(double)));
    }
}

WaveformRenderSignal::~WaveformRenderSignal() {
    qDebug() << this << "~WaveformRenderSignal()";
    delete m_pGain;
}

void WaveformRenderSignal::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
}

void WaveformRenderSignal::newTrack(TrackPointer pTrack) {
    m_pTrack = pTrack;
}

void WaveformRenderSignal::slotUpdateGain(double v) {
    m_fGain = v;
}

void WaveformRenderSignal::setup(QDomNode node) {
    signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    signalColor = WSkinColor::getCorrectColor(signalColor);
}


void WaveformRenderSignal::draw(QPainter *pPainter, QPaintEvent *event,
                                QVector<float> *buffer, double dPlayPos,
                                double rateAdjust) {
    if (buffer == NULL) {
        return;
    }

    const float* baseBuffer = buffer->constData();

    int numBufferSamples = buffer->size();
    int iCurPos = 0;
    iCurPos = (int)(dPlayPos*numBufferSamples);

    if ((iCurPos % 2) != 0)
        iCurPos--;

    pPainter->save();

    pPainter->setPen(signalColor);

    const double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel() * (1.0 + rateAdjust);

    int subpixelWidth = int(m_iWidth * subpixelsPerPixel);

    // If the array is not large enough, expand it.
    // Amortize the cost of this by requesting a factor of 2 more.
    if(m_lines.size() < subpixelWidth) {
        m_lines.resize(2*subpixelWidth);
    }

    // Use the pointer to the QVector internal data to avoid range
    // checks. QVector<QLineF>::operator[] profiled very high. const_cast is
    // naughty but we just want Qt to leave us alone here. WARNING: calling
    // m_lines.data() will copy the entire vector in memory by calling
    // QVector<T>::detach(). QVector<T>::constData() does not do this.
    QLineF* lineData = const_cast<QLineF*>(m_lines.constData());
    int halfw = subpixelWidth/2;
    for(int i=0;i<subpixelWidth;i++) {
        // Start at curPos minus half the waveform viewer
        int thisIndex = iCurPos+2*(i-halfw);
        if(thisIndex >= 0 && (thisIndex+1) < numBufferSamples) {
            float sampl = baseBuffer[thisIndex] * m_fGain * m_iHeight * 0.5f;
            float sampr = -baseBuffer[thisIndex+1] * m_fGain * m_iHeight * 0.5f;
            const qreal xPos = i/subpixelsPerPixel;
            lineData[i].setLine(xPos, sampr, xPos, sampl);
        } else {
            lineData[i].setLine(0,0,0,0);
        }
    }

    // Only draw lines that we have provided
    pPainter->drawLines(lineData, subpixelWidth);

    // Some of the pre-roll is on screen. Draw little triangles to indicate
    // where the pre-roll is located.
    if (iCurPos < 2*halfw) {
        double start_index = 0;
        int end_index = (halfw - iCurPos/2);
        QPolygonF polygon;
        const int polyWidth = 80;
        polygon << QPointF(0, 0)
                << QPointF(-polyWidth/subpixelsPerPixel, -m_iHeight/5)
                << QPointF(-polyWidth/subpixelsPerPixel, m_iHeight/5);
        polygon.translate(end_index/subpixelsPerPixel, 0);

        int index = end_index;
        while (index > start_index) {
            pPainter->drawPolygon(polygon);
            polygon.translate(-polyWidth/subpixelsPerPixel, 0);
            index -= polyWidth;
        }
    }
    pPainter->restore();
}
