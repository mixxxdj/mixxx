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
#include "waveform.h"

#include <QTime>

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
    if(m_pGain != NULL) {
        connect(m_pGain, SIGNAL(valueChanged(double)),
                this, SLOT(slotUpdateGain(double)));
    }

    lowColor.setRgb(134,20,4);
    midColor.setRgb(200,85,237);
    highColor.setRgb(85,169,237);

    elapsed.reserve(100000);
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


void WaveformRenderSignal::draw_old(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double dPlayPos, double rateAdjust) {
    if(buffer == NULL) {
        return;
    }

    float* baseBuffer = buffer->data();

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

void WaveformRenderSignal::draw(QPainter *pPainter, QPaintEvent *event,
                                QVector<float> *buffer, double dPlayPos, double rateAdjust)
{
    //return draw_point(pPainter,event,buffer,dPlayPos,rateAdjust);

    //qDebug() << "frame time" << timer.elapsed();
    timer.start();

    if(buffer == NULL)
        return;

    int numBufferSamples = buffer->size();
    int iCurPos = 0;
    if(dPlayPos >= 0) {
        iCurPos = (int)(dPlayPos*numBufferSamples);
    }

    if((iCurPos % 2) != 0)
        iCurPos--;

    double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel() * (1.0 + rateAdjust);

    int subpixelWidth = int(m_iWidth * subpixelsPerPixel);

    // If the array is not large enough, expand it.
    // Amortize the cost of this by requesting a factor of 2 more.
    if(m_lines.size() < subpixelWidth) {
        m_lines.resize(2*subpixelWidth);
    }

    //QVector<QLineF> linesLow(subpixelWidth), linesMid(subpixelWidth), linesHigh(subpixelWidth);

    const Waveform* waveform = m_pTrack->getWaveForm();

    int h,s,l;
    signalColor.getHsl(&h,&s,&l);
    lowColor = signalColor;
    midColor = QColor::fromHsl( (h-20) % 255,255,100,200);
    highColor =  QColor::fromHsl( h,255,220,200);

    pPainter->save();

    float powerFactor = m_iHeight * m_fGain * 1/255.0f;

    int halfw = subpixelWidth/2;
    for(int i=0;i<subpixelWidth;i++) {
        // Start at curPos minus half the waveform viewer
        int thisIndex = iCurPos+2*(i-halfw);
        if(thisIndex >= 0 && (thisIndex+1) < numBufferSamples) {
            const float xPos = i/subpixelsPerPixel;

            /*
            m_lines[i].setLine( xPos, -waveform->getConstData()[thisIndex] * powerFactor,
                                xPos, waveform->getConstData()[thisIndex+1] * powerFactor);
            linesLow[i].setLine( xPos, -waveform->getConstLowData()[thisIndex] * powerFactor,
                                 xPos, waveform->getConstLowData()[thisIndex+1] * powerFactor);
            linesMid[i].setLine( xPos, -waveform->getConstBandData()[thisIndex] * powerFactor * 2,
                                 xPos, waveform->getConstBandData()[thisIndex+1] * powerFactor * 2);
            linesHigh[i].setLine( xPos, -waveform->getConstHighData()[thisIndex] * powerFactor * 4,
                                  xPos, waveform->getConstHighData()[thisIndex+1] * powerFactor * 4);
                                  */

            pPainter->setPen( lowColor);
            pPainter->drawLine( xPos, (float) - waveform->getLow(thisIndex) * powerFactor,
                                xPos, (float) waveform->getLow(thisIndex+1) * powerFactor);
            pPainter->setPen( lowColor);
            pPainter->drawLine( xPos, (float) - waveform->getMid(thisIndex) * powerFactor * 2.0f,
                                xPos, (float) waveform->getMid(thisIndex+1) * powerFactor * 2.0f);
            pPainter->setPen( highColor);
            pPainter->drawLine( xPos, (float) - waveform->getHigh(thisIndex) * powerFactor * 4.0f,
                                xPos, (float) waveform->getHigh(thisIndex+1) * powerFactor * 4.0f);

        }
        else
        {
            /*
            m_lines[i].setLine(0,0,0,0);
            linesLow[i].setLine(0,0,0,0);
            linesMid[i].setLine(0,0,0,0);
            linesHigh[i].setLine(0,0,0,0);
            */
        }
    }

    /*
    pPainter->save();

    //pPainter->setPen(QColor(200,200,200,255));
    //pPainter->drawLines(m_lines.data(), subpixelWidth);

    signalColor.setAlpha(200);

    int h,s,l;
    signalColor.getHsl(&h,&s,&l);

    pPainter->setPen(signalColor);
    pPainter->drawLines(linesLow.data(), subpixelWidth);

    pPainter->setPen( QColor::fromHsl( (h-20) % 255,255,100,200));
    pPainter->drawLines(linesMid.data(), subpixelWidth);

    pPainter->setPen( QColor::fromHsl( h,255,220,200));
    pPainter->drawLines(linesHigh.data(), subpixelWidth);
    */

    pPainter->restore();

    qDebug() << timer.elapsed();
    //timer.restart();
    //elapsed.push_back(timer.elapsed());
}

void WaveformRenderSignal::draw_point(QPainter *pPainter, QPaintEvent *event, QVector<float> *buffer, double playPos, double rateAdjust)
{
    timer.start();

    if(buffer == NULL)
        return;

    float* baseBuffer = buffer->data();

    int numBufferSamples = buffer->size();
    int currentPosition = 0;
    if( playPos >= 0)
        currentPosition = (int)(playPos*(double)numBufferSamples);

    //double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel() * (1.0 + rateAdjust);
    double subpixelsPerPixel = m_pParent->getSubpixelsPerPixel(); //zoom

    //if((currentPosition % 2) != 0)
    currentPosition -= (currentPosition % (int)(2*subpixelsPerPixel));

    const Waveform* waveform = m_pTrack->getWaveForm();

    int h,s,l;
    signalColor.getHsl(&h,&s,&l);
    lowColor = signalColor;
    midColor = QColor::fromHsl( (h-20) % 255,255,100,200);
    highColor =  QColor::fromHsl( h,255,220,200);

    pPainter->save();

    //signalColor.setAlphaF(1.0);
    pPainter->setPen(signalColor);
    pPainter->setRenderHint(QPainter::Antialiasing, false);

    float powerFactor = (float)m_iHeight * m_fGain / 255.0f;

    //Paint the whole widget once a pixel
    int currentDisplayPosition = 0;
    for( int xPos = 0; xPos < m_iWidth; ++xPos)
    {
        //current pos is the center of the widget
        currentDisplayPosition = currentPosition + subpixelsPerPixel * ( 2 * xPos - m_iWidth);
        if( currentDisplayPosition > 0 && currentDisplayPosition < waveform->size())
        {
            pPainter->setPen( lowColor);
            pPainter->drawLine( xPos, (int)((float) - waveform->getLow(currentDisplayPosition) * powerFactor),
                                xPos, (int)((float) waveform->getLow(currentDisplayPosition+1) * powerFactor));
            pPainter->setPen( lowColor);
            pPainter->drawLine( xPos, (int)((float) - waveform->getMid(currentDisplayPosition) * powerFactor * 2.0f),
                                xPos, (int)((float) waveform->getMid(currentDisplayPosition+1) * powerFactor * 2.0f));
            pPainter->setPen( highColor);
            pPainter->drawLine( xPos, (int)((float) - waveform->getHigh(currentDisplayPosition) * powerFactor * 4.0f),
                                xPos, (int)((float) waveform->getHigh(currentDisplayPosition+1) * powerFactor * 4.0f));
        }
        //else
        //{
        //    pPainter->drawPoint(xPos,m_iHeight/2);
        //}
    }

    pPainter->restore();

    qDebug() << timer.elapsed();

    /*
    QVector<QLineF> linesLow(subpixelWidth), linesMid(subpixelWidth), linesHigh(subpixelWidth);

    QVector<float>& power = *(m_pTrack->getPower());
    QVector<unsigned char>& lowPower = *(m_pTrack->getLowPower());
    QVector<unsigned char>& midPower = *(m_pTrack->getMidPower());
    QVector<unsigned char>& highPower = *(m_pTrack->getHighPower());

    float powerFactor = 1/255.0f;

    int halfw = subpixelWidth/2;
    for(int i=0;i<subpixelWidth;i++) {
        // Start at curPos minus half the waveform viewer
        int thisIndex = currentPosition+2*(i-halfw);
        if(thisIndex >= 0 && (thisIndex+1) < numBufferSamples) {
            const qreal xPos = i/subpixelsPerPixel;

            m_lines[i].setLine( xPos, -power[thisIndex] * m_iHeight, xPos, power[thisIndex+1] * m_iHeight);
            linesLow[i].setLine( xPos, -lowPower[thisIndex] * m_iHeight * powerFactor, xPos, lowPower[thisIndex+1] * m_iHeight * powerFactor);
            linesMid[i].setLine( xPos, -midPower[thisIndex] * m_iHeight * powerFactor, xPos, midPower[thisIndex+1] * m_iHeight * powerFactor);
            linesHigh[i].setLine( xPos, -highPower[thisIndex] * m_iHeight * powerFactor, xPos, highPower[thisIndex+1] * m_iHeight * powerFactor);
        }
        else
        {
            m_lines[i].setLine(0,0,0,0);
            linesLow[i].setLine(0,0,0,0);
            linesMid[i].setLine(0,0,0,0);
            linesHigh[i].setLine(0,0,0,0);
        }
    }

    pPainter->save();

    //pPainter->setPen(QColor(200,200,200,255));
    //pPainter->drawLines(m_lines.data(), subpixelWidth);

    signalColor.setAlphaF(0.8);

    pPainter->setPen(signalColor.darker(150));
    //pPainter->setPen(QColor(255,0,0,150));
    pPainter->drawLines(linesLow.data(), subpixelWidth);

    pPainter->setPen(signalColor);
    //pPainter->setPen(QColor(0,255,0,150));
    pPainter->drawLines(linesMid.data(), subpixelWidth);

    pPainter->setPen( signalColor.lighter(150));
    //pPainter->setPen(QColor(0,0,255,150));
    pPainter->drawLines(linesHigh.data(), subpixelWidth);
*/
}
