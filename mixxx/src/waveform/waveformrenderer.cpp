/**

  A license and other info goes here!

 */

#include <QDebug>
#include <QDomNode>
#include <QImage>
#include <QObject>

#include <time.h>

#include "waveformrenderer.h"
#include "waveformrenderbeat.h"
#include "trackinfoobject.h"
#include "soundsourceproxy.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "wwidget.h"
#include "wskincolor.h"

#define DEFAULT_SECONDS_TO_DISPLAY 4
#define SCALE_TEST 4

WaveformRenderer::WaveformRenderer(const char* group) :
m_iWidth(0),
m_iHeight(0),
m_iMax(0),
m_iMin(0),
m_iNumSamples(0),
bgColor(0,0,0),
signalColor(255,255,255),
colorMarker(255,255,255),
colorBeat(255,255,255),
colorCue(255,255,255),
m_pPixmap(0),
m_lines(0),
m_iDesiredSecondsToDisplay(DEFAULT_SECONDS_TO_DISPLAY),
m_pImage(),
m_dPlayPos(0),
m_dPlayPosOld(-1),
m_iPlayPosTime(-1),
m_iPlayPosTimeOld(-1)
{
    m_pPlayPos = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group,"visual_playposition")));
    connect(m_pPlayPos, SIGNAL(valueChanged(double)), this, SLOT(slotUpdatePlayPos(double)));

    m_pRenderBeat = new WaveformRenderBeat(group, this);
    //    if(!m_pPlayPos)
    //        qFatal() << "playposition control object couldn't be created";

    m_pCOVisualResample = new ControlObject(ConfigKey(group, "VisualResample"));
}


WaveformRenderer::~WaveformRenderer() {
    if(m_pCOVisualResample)
        delete m_pCOVisualResample;
    m_pCOVisualResample = NULL;

    if(m_pPlayPos)
        delete m_pPlayPos;
    m_pPlayPos = NULL;

    if(m_pRenderBeat)
        delete m_pRenderBeat;
    m_pRenderBeat = NULL;

}

void WaveformRenderer::slotUpdatePlayPos(double v) {
    m_iPlayPosTimeOld = m_iPlayPosTime;
    m_dPlayPosOld = m_dPlayPos;    
    m_dPlayPos = v;
    m_iPlayPosTime = clock();
}

void WaveformRenderer::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;
    m_lines.resize(w*SCALE_TEST);
    
    m_pRenderBeat->resize(w,h);

    setupControlObjects();
}

void WaveformRenderer::setupControlObjects() {

    // the resample rate is the number of seconds that correspond to one pixel
    // on the visual waveform display.

    // the reason for using seconds is that we do not know the
    // sample rate of the song that will be loaded.

    // we calculate this as follows:

    // secondsPerPixel = desiredSecondsToDisplay / m_iWidth

    // for now just send the width.. meh
    double secondsPerPixel = double(m_iDesiredSecondsToDisplay)/m_iWidth;
    //m_pCOVisualResample->set(secondsPerPixel);

    m_pCOVisualResample->set(m_iWidth);

    qDebug() << "WaveformRenderer::setupControlObjects - VisualResample: " << secondsPerPixel;

}

void WaveformRenderer::setup(QDomNode node) {

    bgColor.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
    bgColor = WSkinColor::getCorrectColor(bgColor);

    qDebug() << "Got bgColor " << bgColor;
    
    signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    signalColor = WSkinColor::getCorrectColor(signalColor);

    qDebug() << "Got signalColor " << signalColor;

    colorMarker.setNamedColor(WWidget::selectNodeQString(node, "MarkerColor"));
    colorMarker = WSkinColor::getCorrectColor(colorMarker);

    colorBeat.setNamedColor(WWidget::selectNodeQString(node, "BeatColor"));
    colorBeat = WSkinColor::getCorrectColor(colorBeat);

    colorCue.setNamedColor(WWidget::selectNodeQString(node, "CueColor"));
    colorCue = WSkinColor::getCorrectColor(colorCue);

    m_pRenderBeat->setup(node);
}


void WaveformRenderer::precomputePixmap() {
    if(m_pSampleBuffer == NULL || m_pPixmap != NULL || m_iNumSamples == 0 || !m_pImage.isNull())
        return;

    qDebug() << "Generating a image!";

    int monoSamples = (m_iNumSamples >> 3);
    qDebug() << monoSamples << " samples for qimage";
    QImage qi(monoSamples, m_iHeight, QImage::Format_RGB32);
    
    QPainter paint;
    paint.begin(&qi);

    paint.fillRect(qi.rect(), QBrush(QColor(255,0,0)));//bgColor));//QColor(0,0,0)));
    paint.setPen(QColor(0,255,0));//signalColor);//QColor(0,255,0));
    qDebug() << "height " << m_iHeight;
    paint.translate(0,m_iHeight/2);
    paint.scale(1.0,-1.0);
    paint.drawLine(QLine(0,0,monoSamples,0));
    //for (int i=0;i<100;i++) {
        //paint.drawLine(QLine(i,0,i,m_iHeight/2));
        //paint.drawLine(QLine(i,0,i,m_iHeight/2));
    //}
    
    for(int i=0;i<monoSamples;i++) {
        //SAMPLE sampl = (*m_pSampleBuffer)[i*2];
        //SAMPLE sampr = (*m_pSampleBuffer)[i*2+1];

        //paint.drawLine(QLine(i,-5, i, 0));
        paint.drawLine(QLine(i,2, i, 80));
        //paint.drawLine(QLine(i,-sampr,i,sampl));
    }
    paint.end();
    qDebug() << "done with image";
    qi.save("/home/rryan/foo.bmp", "BMP", 100);
    m_pImage = qi;

    
    return;

    /*
    qDebug() << "Generating a pixmap!";

    // Now generate a pixmap of this
    QPixmap *pm = new QPixmap(m_iNumSamples/2, m_iHeight);

    if(pm->isNull()) {
        qDebug() << "Built a null pixmap, WTF!";
    } else {
        qDebug() << " Build a pixmap " << pm->size();
    }
    
    QPainter paint;
    paint.begin(pm);

    qDebug() << "Wave Precomp: BG: " << bgColor << " FG:" << signalColor;
    paint.fillRect(pm->rect(), QBrush(bgColor));//QColor(0,0,0)));
    paint.setPen(signalColor);//QColor(0,255,0));

    paint.translate(0,m_iHeight/2);
    paint.scale(1.0,-1.0);
    //paint.drawLine(QLine(0,0,resultSamples/2,0));
    
    for(int i=0;i<m_iNumSamples/2;i++) {
        SAMPLE sampl = (*m_pSampleBuffer)[i*2];
        SAMPLE sampr = (*m_pSampleBuffer)[i*2+1];

        //paint.drawLine(QLine(i,-15, i, 15));
        paint.drawLine(QLine(i,-sampr,i,sampl));
    }
    paint.end();
    
    m_pPixmap = pm;
    */
}

bool WaveformRenderer::fetchWaveformFromTrack() {
    
    if(!m_pTrack)
        return false;

    QVector<float> *buffer = m_pTrack->getVisualWaveform();

    if(buffer == NULL)
        return false;

    m_pSampleBuffer = buffer;
    m_iNumSamples = buffer->size();

    return true;
}
    


void WaveformRenderer::drawSignalLines(QPainter *pPainter,double playpos) {
    
    if(m_pSampleBuffer == NULL) {
        return;
    }

    int iCurPos = 0;
    if(m_dPlayPos != -1) {
        iCurPos = (int)(playpos*m_iNumSamples);
    }
        
    if((iCurPos % 2) != 0)
        iCurPos--;

    pPainter->save();

    pPainter->scale(1.0/float(SCALE_TEST),m_iHeight*0.40);
    int halfw = m_iWidth*SCALE_TEST/2;
    for(int i=0;i<m_iWidth*SCALE_TEST;i++) {
        int thisIndex = iCurPos+2*(i-halfw);
        if(thisIndex >= 0 && (thisIndex+1) < m_iNumSamples) {
            float sampl = (*m_pSampleBuffer)[thisIndex];
            float sampr = (*m_pSampleBuffer)[thisIndex+1];
            m_lines[i] = QLineF(i,-sampr,i,sampl);
        } else {
            m_lines[i] = QLineF(0,0,0,0);
        }
    }

    pPainter->drawLines(m_lines);

    pPainter->restore();
}

void WaveformRenderer::drawSignalPixmap(QPainter *pPainter) {

    //if(m_pPixmap == NULL)
    //return;
    //if(m_pImage == NULL)
    //return;
    if(m_pImage.isNull())
        return;
    
    //double dCurPos = m_pPlayPos->get();
    int iCurPos = (int)(m_dPlayPos*m_pImage.width());
        
    int halfw = m_iWidth/2;
    int halfh = m_iHeight/2;



    //int totalHeight = m_pPixmap->height();
    //int totalWidth = m_pPixmap->width();
    int totalHeight = m_pImage.height();
    int totalWidth = m_pImage.width();
    int width = m_iWidth;
    int height = m_iHeight;
    // widths and heights of the two rects should be the same:
    // m_iWidth - 0 = iCurPos + halfw - iCurPos + halfw = m_iWidth (if even)
    // -halfh-halfh = -halfh-halfh

    int sx=iCurPos-halfw;
    int sy=0;
    int tx=0;
    int ty=0;

    if(sx < 0) {
        sx = 0;
        width = iCurPos + halfw;
        tx = m_iWidth - width;
    } else if(sx + width >= totalWidth) {
        //width = (iCurPos - sx) + (totalWidth-iCurPos);
        width = halfw + totalWidth - iCurPos;
    }

    QRect target(tx,ty,width,height);
    QRect source(sx,sy,width,height);

    //qDebug() << "target:" << target;
    //qDebug() << "source:" << source;
    pPainter->setPen(signalColor);
    //pPainter->drawPixmap(target,*m_pPixmap,source);
    pPainter->drawImage(target, m_pImage, source);

}



void WaveformRenderer::draw(QPainter* pPainter, QPaintEvent *pEvent) {
    double playposadjust = 0;

    /*
    int latency = m_iPlayPosTime - m_iPlayPosTimeOld;
    int timeelapsed = clock() - m_iPlayPosTime;
    double timeratio = 0;
    if(latency != 0)
        timeratio = double(timeelapsed) / double(latency);
    if(timeratio > 1.0)
        timeratio = 1.0;

    if(m_dPlayPosOld != -1) {
        playposadjust = (m_dPlayPos - m_dPlayPosOld) * timeratio;
    }
    */
    double playpos = m_dPlayPos + playposadjust;
    
    
    
    pPainter->fillRect(pEvent->rect(), QBrush(bgColor));
    pPainter->setPen(signalColor);
    
    if(m_iWidth == 0 || m_iHeight == 0)
        return;
    
    if(m_pSampleBuffer == NULL) {
        fetchWaveformFromTrack();
        if(m_pSampleBuffer != NULL)
            qDebug() << "Received waveform from track";
    }

    //if(m_pPixmap == NULL) {
        //precomputePixmap();
    //}

    //drawSignalPixmap(pPainter);

    // Translate our coordinate frame from (0,0) at top left
    // to (0,0) at left, center.
    pPainter->translate(0.0,m_iHeight/2.0);
    // Now scale so that positive-y points up.
    pPainter->scale(1.0,-1.0);
    
    drawSignalLines(pPainter,playpos);

    m_pRenderBeat->draw(pPainter,pEvent, m_pSampleBuffer, playpos);
    
    // Draw various markers.
    pPainter->setPen(colorMarker);
    
    // Draw the center horizontal line
    pPainter->drawLine(QLine(0,0,m_iWidth,0));

    // Draw the center vertical line
    pPainter->drawLine(QLineF(m_iWidth/2.0,m_iHeight/2.0,m_iWidth/2.0,-m_iHeight/2.0));

}

void WaveformRenderer::newTrack(TrackInfoObject* pTrack) {
    m_pTrack = pTrack;
    m_pSampleBuffer = NULL;
    m_iNumSamples = 0;
    m_dPlayPos = 0;
    m_dPlayPosOld = 0;
    m_pRenderBeat->newTrack(pTrack);
}

int WaveformRenderer::getDesiredSecondsToDisplay() {
    return m_iDesiredSecondsToDisplay;
}

void WaveformRenderer::setDesiredSecondsToDisplay(int secondsToDisplay) {
    m_iDesiredSecondsToDisplay = secondsToDisplay;
}
