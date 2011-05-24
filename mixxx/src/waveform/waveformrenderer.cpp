#include <QDebug>
#include <QDomNode>
#include <QImage>
#include <QListIterator>
#include <QObject>

#include <time.h>

#include "mathstuff.h"
#include "waveformrenderer.h"
#include "waveformrenderbackground.h"
#include "waveformrenderbeat.h"
#include "waveformrendermark.h"
#include "waveformrendermarkrange.h"
#include "waveformrendersignal.h"
#include "waveformrendersignalpixmap.h"
#include "trackinfoobject.h"
#include "soundsourceproxy.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wwidget.h"
#include "widget/wskincolor.h"

#define INTERPOLATION 0

#define DEFAULT_SUBPIXELS_PER_PIXEL 4
#define DEFAULT_PIXELS_PER_SECOND 100

#define RATE_INCREMENT 0.015

void WaveformRenderer::run() {
    double msecs_old = 0, msecs_elapsed = 0;

    while(!m_bQuit) {

        if(m_iLatency != 0 && m_dPlayPos != -1 && m_dPlayPosOld != -1 && m_iNumSamples != 0) {
            QTime now = QTime::currentTime();
            double msecs_elapsed = m_playPosTime.msecsTo(now);
            double timeratio = double(msecs_elapsed) / m_iLatency;
            double adjust = (m_dPlayPos - m_dPlayPosOld) * math_min(1.0f, timeratio);
            m_dPlayPosAdjust = adjust;
        }

        QThread::msleep(6);
    }
}

WaveformRenderer::WaveformRenderer(const char* group) :
    QThread(),
    m_pGroup(group),
    m_iWidth(0),
    m_iHeight(0),
    bgColor(0,0,0),
    signalColor(255,255,255),
    colorMarker(255,255,255),
    colorBeat(255,255,255),
    colorCue(255,255,255),
    m_iNumSamples(0),
    m_iPlayPosTime(-1),
    m_iPlayPosTimeOld(-1),
    m_dPlayPos(0),
    m_dPlayPosOld(-1),
    m_dRate(0),
    m_dRateRange(0),
    m_dRateDir(0),
    m_iRateAdjusting(0),
    m_iDupes(0),
    m_dPlayPosAdjust(0),
    m_iLatency(0),
    m_pSampleBuffer(NULL),
    m_pPixmap(NULL),
    m_pImage(),
    m_iSubpixelsPerPixel(DEFAULT_SUBPIXELS_PER_PIXEL),
    m_iPixelsPerSecond(DEFAULT_PIXELS_PER_SECOND),
    m_pTrack(NULL),
    m_bQuit(false)
{
    m_pPlayPos = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(group,"visual_playposition")));
    if(m_pPlayPos != NULL)
        connect(m_pPlayPos, SIGNAL(valueChanged(double)),
                this, SLOT(slotUpdatePlayPos(double)));


    m_pLatency = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey("[Master]","latency")));
    if(m_pLatency != NULL)
        connect(m_pLatency, SIGNAL(valueChanged(double)),
                this, SLOT(slotUpdateLatency(double)));

    m_pRenderBackground = new WaveformRenderBackground(group, this);
    m_pRenderSignal = new WaveformRenderSignal(group, this);
    m_pRenderSignalPixmap = new WaveformRenderSignalPixmap(group, this);
    m_pRenderBeat = new WaveformRenderBeat(group, this);

    m_pCOVisualResample = new ControlObject(ConfigKey(group, "VisualResample"));

    m_pRate = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(group, "rate")));
    if(m_pRate != NULL) {
        connect(m_pRate, SIGNAL(valueChanged(double)),
                this, SLOT(slotUpdateRate(double)));
    }

    m_pRateRange = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(group, "rateRange")));
    if(m_pRateRange != NULL) {
        connect(m_pRateRange, SIGNAL(valueChanged(double)),
                this, SLOT(slotUpdateRateRange(double)));
    }

    m_pRateDir = new ControlObjectThreadMain(
        ControlObject::getControl(ConfigKey(group, "rate_dir")));
    if (m_pRateDir) {
        connect(m_pRateDir, SIGNAL(valueChanged(double)),
                this, SLOT(slotUpdateRateDir(double)));
    }

    if(0)
        start();
}


WaveformRenderer::~WaveformRenderer() {
    qDebug() << this << "~WaveformRenderer()";

    // Wait for the thread to quit
    m_bQuit = true;
    QThread::wait();

    if(m_pRenderBackground)
        delete m_pRenderBackground;
    m_pRenderBackground = NULL;

    if(m_pRenderSignalPixmap)
        delete m_pRenderSignalPixmap;
    m_pRenderSignalPixmap = NULL;

    if(m_pRenderSignal)
        delete m_pRenderSignal;
    m_pRenderSignal = NULL;

    if(m_pRenderBeat)
        delete m_pRenderBeat;
    m_pRenderBeat = NULL;

    QMutableListIterator<RenderObject*> iter(m_renderObjects);
    while (iter.hasNext()) {
        RenderObject* ro = iter.next();
        iter.remove();
        delete ro;
    }

    if(m_pCOVisualResample)
        delete m_pCOVisualResample;
    m_pCOVisualResample = NULL;

    if (m_pPlayPos)
        delete m_pPlayPos;
    m_pPlayPos = NULL;

    if (m_pLatency)
        delete m_pLatency;
    m_pLatency = NULL;;

    if(m_pRate)
        delete m_pRate;
    m_pRate = NULL;

    if(m_pRateRange)
        delete m_pRateRange;
    m_pRateRange = NULL;

    if(m_pRateDir)
        delete m_pRateDir;
    m_pRateDir = NULL;

    if(m_pPlayPos)
        delete m_pPlayPos;
    m_pPlayPos = NULL;
}

void WaveformRenderer::slotUpdatePlayPos(double v) {
    m_iPlayPosTimeOld = m_iPlayPosTime;
    //m_playPosTimeOld = m_playPosTime;
    m_dPlayPosOld = m_dPlayPos;
    m_dPlayPos = v;
    m_iPlayPosTime = clock();
    //m_playPosTime = QTime::currentTime();

    m_iDupes = 0;
    m_dPlayPosAdjust = 0;
}

void WaveformRenderer::slotUpdateRate(double v) {
    m_dTargetRate = v;
}

void WaveformRenderer::slotUpdateRateRange(double v) {
    m_dRateRange = v;
}

void WaveformRenderer::slotUpdateRateDir(double v) {
    m_dRateDir = v;
}

void WaveformRenderer::slotUpdateLatency(double v) {
    m_iLatency = v;
}

void WaveformRenderer::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;

    setupControlObjects();

    // Notify children that we've been resized
    m_pRenderBackground->resize(w,h);
    m_pRenderSignal->resize(w,h);
    m_pRenderSignalPixmap->resize(w,h);
    m_pRenderBeat->resize(w,h);

    QListIterator<RenderObject*> iter(m_renderObjects);
    while (iter.hasNext()) {
        RenderObject* ro = iter.next();
        ro->resize(w,h);
    }
}

void WaveformRenderer::setupControlObjects() {

    // the resample rate is the number of samples that correspond to one downsample

    // This set of restrictions provides for a downsampling setup like this:

    // Let a sample be a sample in the original song.
    // Let a downsample be a sample in the downsampled buffer
    // Let a pixel be a pixel on the screen.

    // W samples -> X downsamples -> Y pixels

    // We start with the restriction that we desire 1 second of
    // 'raw' information to be contained within Z real pixels of screen space.

    // 1) 1 second / z pixels = f samples / z pixels  = (f/z) samples per pixel

    // The size of the buffer we interact with is the number of downsamples

    // The ratio of samples to downsamples is N : 1
    // The ratio of downsamples to pixels is M : 1

    // Therefore the ratio of samples to pixels is MN : 1

    // Or in other words, we have MN samples per pixel

    // 2) MN samples / pixel

    // We combine 1 and 2 into one constraint:

    // (f/z) = mn, or  f = m * n * z

    // REQUIRE : M * N * Z = F
    // M : DOWNSAMPLES PER PIXEL
    // N : SAMPLES PER DOWNSAMPLE
    // F : SAMPLE RATE OF SONG
    // Z : THE USER SEES 1 SECOND OF DATA IN Z PIXELS

    // Solving for N, the number of samples in our downsample buffer,
    // we get : N = F / (M*Z)

    // We don't know F, so we're going to transmit M*Z

    double m = m_iSubpixelsPerPixel; // M DOWNSAMPLES PER PIXEL
    double z = m_iPixelsPerSecond; // Z PIXELS REPRESENTS 1 SECOND OF DATA

    m_pCOVisualResample->set(m*z);

    //qDebug() << "WaveformRenderer::setupControlObjects - VisualResample: " << m*z;

}

void WaveformRenderer::setup(QDomNode node) {

    bgColor.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
    bgColor = WSkinColor::getCorrectColor(bgColor);

    signalColor.setNamedColor(WWidget::selectNodeQString(node, "SignalColor"));
    signalColor = WSkinColor::getCorrectColor(signalColor);

    colorMarker.setNamedColor(WWidget::selectNodeQString(node, "MarkerColor"));
    colorMarker = WSkinColor::getCorrectColor(colorMarker);

    colorBeat.setNamedColor(WWidget::selectNodeQString(node, "BeatColor"));
    colorBeat = WSkinColor::getCorrectColor(colorBeat);

    colorCue.setNamedColor(WWidget::selectNodeQString(node, "CueColor"));
    colorCue = WSkinColor::getCorrectColor(colorCue);

    while (m_renderObjects.size() > 0) {
        RenderObject* ro = m_renderObjects.takeFirst();
        delete ro;
    }

    // Process any <Mark> nodes
    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        RenderObject* pRenderObject = NULL;
        if (child.nodeName() == "Mark") {
            pRenderObject = new WaveformRenderMark(m_pGroup, this);
        } else if(child.nodeName() == "MarkRange") {
            pRenderObject = new WaveformRenderMarkRange(m_pGroup, this);
        }
        if (pRenderObject != NULL) {
            if (m_pTrack != NULL)
                pRenderObject->newTrack(m_pTrack);
            pRenderObject->setup(child);
            m_renderObjects.push_back(pRenderObject);
        }
        child = child.nextSibling();
    }

    m_pRenderBackground->setup(node);
    m_pRenderSignal->setup(node);
    m_pRenderSignalPixmap->setup(node);
    m_pRenderBeat->setup(node);
}


void WaveformRenderer::precomputePixmap() {
    if(m_pSampleBuffer == NULL || m_iNumSamples == 0 || !m_pImage.isNull())
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

void WaveformRenderer::drawSignalPixmap(QPainter *pPainter) {


    //if(m_pImage == NULL)
    //return;
    if(m_pImage.isNull())
        return;

    //double dCurPos = m_pPlayPos->get();
    int iCurPos = (int)(m_dPlayPos*m_pImage.width());

    int halfw = m_iWidth/2;
    int halfh = m_iHeight/2;

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

    pPainter->drawImage(target, m_pImage, source);

}

void WaveformRenderer::draw(QPainter* pPainter, QPaintEvent *pEvent) {
    double playposadjust = 0;

    if(m_iWidth == 0 || m_iHeight == 0)
        return;


    /*
    if(m_dPlayPos != -1 && m_dPlayPosOld != -1 && m_iNumSamples != 0) {
        static double elatency = ControlObject::getControl(ConfigKey("[Master]","latency"))->get();
        double latency = elatency;
        latency *= 4;
        latency *= CLOCKS_PER_SEC / 1000.0;

        //int latency = m_iPlayPosTime - m_iPlayPosTimeOld;
        double timeelapsed = (clock() - m_iPlayPosTime);
        double timeratio = 0;
        if(latency != 0)
            timeratio = double(timeelapsed) / double(latency);
        if(timeratio > 1.0)
            timeratio = 1.0;

        double timerun = m_iPlayPosTime - m_iPlayPosTimeOld;


        playposadjust = ((m_dPlayPos*m_iNumSamples) - (m_dPlayPosOld*m_iNumSamples)) * timeelapsed;
        playposadjust /= (latency*m_iNumSamples);

        //qDebug() << m_dPlayPos - m_dPlayPosOld << " " << timerun;

        //qDebug() << "ppold " << m_dPlayPosOld << " pp " << m_dPlayPos << " timeratio " << timeratio;
        //qDebug() << "timee" << timeelapsed <<  "playpoadj" << playposadjust;
    }
    */
    m_iDupes++;

    double playpos = m_dPlayPos + m_dPlayPosAdjust;

    //qDebug() << m_dPlayPosAdjust;

    // Gradually stretch the waveform
    if (fabs(m_dTargetRate - m_dRate) > RATE_INCREMENT)
    {
        if ((m_dTargetRate - m_dRate) > 0)
        {
            m_iRateAdjusting = m_iRateAdjusting > 0 ? m_iRateAdjusting + 1 : 1;
            m_dRate = math_min(m_dTargetRate, m_dRate + RATE_INCREMENT * pow(m_iRateAdjusting, 2) / 80);
        }
        else
        {
            m_iRateAdjusting = m_iRateAdjusting < 0 ? m_iRateAdjusting - 1 : -1;
            m_dRate = math_max(m_dTargetRate, m_dRate - RATE_INCREMENT * pow(m_iRateAdjusting, 2) / 80);
        }
    }
    else
    {
        m_iRateAdjusting = 0;
        m_dRate = m_dTargetRate;
    }
        
    // Limit our rate adjustment to < 99%, "Bad Things" might happen otherwise.
    double rateAdjust = m_dRateDir * math_min(0.99, m_dRate * m_dRateRange);

    if(m_pSampleBuffer == NULL) {
        fetchWaveformFromTrack();
    }

    m_pRenderBackground->draw(pPainter, pEvent, m_pSampleBuffer, playpos, rateAdjust);

    pPainter->setPen(signalColor);

    //m_pRenderSignalPixmap->draw(pPainter, pEvent, m_pSampleBuffer, playpos, rateAdjust);
    // Translate our coordinate frame from (0,0) at top left
    // to (0,0) at left, center. All the subrenderers expect this.
    pPainter->translate(0.0,m_iHeight/2.0);

    // Now scale so that positive-y points up.
    pPainter->scale(1.0,-1.0);

    // Draw the center horizontal line under the signal.
    pPainter->drawLine(QLine(0,0,m_iWidth,0));

    m_pRenderSignal->draw(pPainter, pEvent, m_pSampleBuffer, playpos, rateAdjust);

    // Draw various markers.
    m_pRenderBeat->draw(pPainter, pEvent, m_pSampleBuffer, playpos, rateAdjust);

    QListIterator<RenderObject*> iter(m_renderObjects);
    while (iter.hasNext()) {
        RenderObject* ro = iter.next();
        ro->draw(pPainter, pEvent, m_pSampleBuffer, playpos, rateAdjust);
    }

    pPainter->setPen(colorMarker);

    // Draw the center vertical line
    pPainter->drawLine(QLineF(m_iWidth/2.0,m_iHeight/2.0,m_iWidth/2.0,-m_iHeight/2.0));

}

void WaveformRenderer::slotUnloadTrack(TrackPointer pTrack) {
    // All RenderObject's must support newTrack() calls with NULL
    slotNewTrack(TrackPointer());
}

void WaveformRenderer::slotNewTrack(TrackPointer pTrack) {

    m_pTrack = pTrack;
    m_pSampleBuffer = NULL;
    m_iNumSamples = 0;
    m_dPlayPos = 0;
    m_dPlayPosOld = 0;

    m_pRenderBackground->newTrack(pTrack);
    m_pRenderSignal->newTrack(pTrack);
    m_pRenderSignalPixmap->newTrack(pTrack);
    m_pRenderBeat->newTrack(pTrack);

    QListIterator<RenderObject*> iter(m_renderObjects);
    while (iter.hasNext()) {
        RenderObject* ro = iter.next();
        ro->newTrack(pTrack);
    }
}

int WaveformRenderer::getPixelsPerSecond() {
    return m_iPixelsPerSecond;
}

int WaveformRenderer::getSubpixelsPerPixel() {
    return m_iSubpixelsPerPixel;
}
