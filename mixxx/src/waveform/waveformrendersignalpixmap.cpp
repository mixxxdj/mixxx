
#include <QDebug>
#include <QColor>
#include <QDomNode>
#include <QPaintEvent>
#include <QPainter>
#include <QObject>
#include <QVector>
#include <QLine>

#include "waveformrendersignalpixmap.h"
#include "waveformrenderer.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "trackinfoobject.h"

WaveformRenderSignalPixmap::WaveformRenderSignalPixmap(const char* group, WaveformRenderer *parent)
        : m_iWidth(0),
          m_iHeight(0),
          m_pParent(parent),
          m_lines(0),
          signalColor(255,255,255),
          m_screenPixmap(),
          m_iLastPlaypos(0) {
}

WaveformRenderSignalPixmap::~WaveformRenderSignalPixmap() {

}

void WaveformRenderSignalPixmap::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
    m_screenPixmap = QPixmap(w,h);
    m_screenPixmap.fill(QColor(0,0,0,0));
}

void WaveformRenderSignalPixmap::newTrack(TrackPointer pTrack) {
    m_pTrack = pTrack;
}

void WaveformRenderSignalPixmap::setup(QDomNode node) {
    signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    signalColor = WSkinColor::getCorrectColor(signalColor);
}

void WaveformRenderSignalPixmap::draw(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double dPlayPos, double rateAdjust) {
    if(buffer == NULL)
        return;

    updatePixmap(buffer, dPlayPos, rateAdjust);

    pPainter->drawPixmap(m_screenPixmap.rect(), m_screenPixmap, event->rect());
}

void WaveformRenderSignalPixmap::updatePixmap(QVector<float> *buffer, double dPlayPos, double rateAdjust) {


    int numBufferSamples = buffer->size();
    int iCurPos = 0;
    if(dPlayPos >= 0) {
        iCurPos = (int)(dPlayPos*numBufferSamples);
    }

    if((iCurPos % 2) != 0)
        iCurPos--;

    double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel() * (1.0 + rateAdjust);
    int subpixelWidth = int(m_iWidth * subpixelsPerPixel);

    int pixelsToDraw = iCurPos - m_iLastPlaypos;
    int startPixel, endPixel;

    QRectF oldRect;
    QPointF oldPoint;
    if(abs(pixelsToDraw) > subpixelWidth) {
        startPixel = 0;
        endPixel = subpixelWidth;
    } else if(pixelsToDraw > 0) {
        // going forward
        startPixel = subpixelWidth - pixelsToDraw;
        endPixel = subpixelWidth;
        //oldRect = QRectF(QPointF(pixelsToDraw,1.0f), QPointF(endPixel, -1.0f));
        oldPoint = QPointF(-pixelsToDraw,1.0f);
    } else if(pixelsToDraw < 0) {
        // going backward
        startPixel = 0;
        endPixel = -pixelsToDraw;
        //oldRect = QRectF(QPointF(0,1.0f), QPointF(subpixelWidth+pixelsToDraw, -1.0f));
        oldPoint = QPointF(pixelsToDraw,1.0f);
    } else {
        // Nothing changed, so we don't have to do anything.
        return;
    }

    QRectF newRect = QRectF(QPointF(startPixel,1.0), QPointF(endPixel, -1.0));
    QPainter painter;
    painter.begin(&m_screenPixmap);
    painter.setPen(signalColor);
    painter.setBackground(QBrush(QColor(0,0,0,0)));
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setBackgroundMode(Qt::TransparentMode);



    // Translate our coordinate frame from (0,0) at top left
    // to (0,0) at left, center. All the subrenderers expect this.
    painter.translate(0.0,m_iHeight/2.0);

    // Now scale so that positive-y points up.
    painter.scale(1.0,-1.0);


    //qDebug() << "startPixel " << startPixel;
    //qDebug() << "endPixel " << endPixel;

    painter.scale(1.0/subpixelsPerPixel,m_iHeight*0.40);

    // If the array is not large enough, expand it.
    // Amortize the cost of this by requesting a factor of 2 more.
    if(m_lines.size() < subpixelWidth) {
        m_lines.resize(2*subpixelWidth);
    }


    painter.drawPixmap(oldPoint, m_screenPixmap);
    painter.eraseRect(newRect);

    //painter.fillRect(newRect, QColor(0,0,0,0));

    int halfw = subpixelWidth/2;
    for(int i=startPixel;i<endPixel;i++) {
        // Start at curPos minus half the waveform viewer
        int thisIndex = iCurPos+2*(i-halfw);
        if(thisIndex >= 0 && (thisIndex+1) < numBufferSamples) {
            float sampl = (*buffer)[thisIndex];
            float sampr = (*buffer)[thisIndex+1];
            m_lines[i] = QLineF(i,-sampr,i,sampl);
        } else {
            m_lines[i] = QLineF(0,0,0,0);
        }
    }

    // Only draw lines that we have provided
    painter.drawLines(m_lines.data()+startPixel, endPixel-startPixel);

    painter.end();

    m_iLastPlaypos = iCurPos;

}
