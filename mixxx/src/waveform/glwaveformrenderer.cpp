/**

  A license and other info goes here!

 */

#include <QDebug>
#include <QDomNode>
#include <QImage>
#include <QObject>
#include <qgl.h>

#include "glwaveformrenderer.h"
#include "waveformrenderbeat.h"
#include "trackinfoobject.h"
#include "soundsourceproxy.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "widget/wwidget.h"
#include "widget/wskincolor.h"

#define DEFAULT_SECONDS_TO_DISPLAY 4
#define SCALE_TEST 4

GLWaveformRenderer::GLWaveformRenderer(const char* group) :
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
m_iDesiredSecondsToDisplay(DEFAULT_SECONDS_TO_DISPLAY),
m_pSampleBuffer(NULL),
m_pInternalBuffer(NULL),
m_iInternalBufferSize(0)
{
    m_pPlayPos = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group,"playposition")));
    connect(m_pPlayPos, SIGNAL(valueChanged(double)), this, SLOT(slotUpdatePlayPos(double)));

    m_pCOVisualResample = new ControlObject(ConfigKey(group,"VisualResample"));
    m_pCOVerticalScale = new ControlObject(ConfigKey(group, "VisualVerticalScale"));
}


GLWaveformRenderer::~GLWaveformRenderer() {
    if(m_pCOVisualResample)
        delete m_pCOVisualResample;
    m_pCOVisualResample = NULL;

    if(m_pCOVerticalScale)
        delete m_pCOVerticalScale;
    m_pCOVerticalScale = NULL;

    if(m_pPlayPos)
        delete m_pPlayPos;
    m_pPlayPos = NULL;

}

void GLWaveformRenderer::slotUpdatePlayPos(double v) {
    m_dPlayPos = v;
}


void GLWaveformRenderer::resize(int w, int h) {
    m_iWidth = w;
    m_iHeight = h;

    m_pInternalBuffer = new GLfloat[w*3*5];
    m_iInternalBufferSize = w*5;

    for(int i=0; i<w*5; i++) {
        m_pInternalBuffer[i*3+0] = i-10;
        m_pInternalBuffer[i*3+1] = 0.5;
        m_pInternalBuffer[i*3+2] = 1.0;


    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1.0, 1.0, 1000.0);
    glViewport(0,0,w,h);

    gluLookAt(0,0,15.0, // look along z-axis
              0,0,0,  // from the origin
              0,1.0,0); // with y being 'up'

    setupControlObjects();
}

void GLWaveformRenderer::setupControlObjects() {

    // the max positive value of a sample is 32767, (2**15-1)
    // we want the waveforms normalized so that 0-32767 maps to 0-m_iHeight/2
    // (m_iHeight/2) = (32767) / x  =>  x = 32767/(m_iHeight/2)
    int verticalScale = ((1<<15)-1)*2/ m_iHeight;
    m_pCOVerticalScale->set(verticalScale);


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

    qDebug() << "GLWaveformRenderer::setupControlObjects - VisualResample: " << secondsPerPixel << " VerticalScale: " << verticalScale;

}

void GLWaveformRenderer::setup(QDomNode node) {

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

}


void GLWaveformRenderer::precomputePixmap() {

}

bool GLWaveformRenderer::fetchWaveformFromTrack() {

    if(!m_pTrack)
        return false;

    QVector<float> *buffer = m_pTrack->getVisualWaveform();

    if(buffer == NULL)
        return false;

    m_pSampleBuffer = buffer;
    m_iNumSamples = buffer->size();

    return true;
}



void GLWaveformRenderer::drawSignalLines() {

    if(m_pSampleBuffer == NULL) {
            return;
    }

    int iCurPos = 0;
    if(m_dPlayPos != -1) {
        iCurPos = (int)(m_dPlayPos*m_iNumSamples);
    }

    if((iCurPos % 2) != 0)
        iCurPos--;


    int halfw = m_iInternalBufferSize/2;

    for(int i=0;i<m_iInternalBufferSize;i++) {
        int thisIndex = iCurPos+2*(i-halfw);
        if(thisIndex >= 0 && (thisIndex+1) < m_iNumSamples) {
            float sampl = (*m_pSampleBuffer)[thisIndex];
            float sampr = (*m_pSampleBuffer)[thisIndex+1];

            m_pInternalBuffer[i*3+1] = sampl;
        }


    }

    /*
    pPainter->scale(1.0/float(SCALE_TEST),m_iHeight*0.40);
    int halfw = m_iWidth*SCALE_TEST/2;
    for(int i=0;i<m_iWidth*SCALE_TEST;i++) {
        //pPainter->drawLine(QPoint(i,0), QPoint(i, i/sca));
        int thisIndex = iCurPos+2*(i-halfw);
        if(thisIndex >= 0 && (thisIndex+1) < m_iNumSamples) {
            float sampl = (*m_pSampleBuffer)[thisIndex];
            float sampr = (*m_pSampleBuffer)[thisIndex+1];

            // Old cruft
            //pPainter->drawLine(QPoint(i,m_iHeight/4), QPoint(i,m_iHeight/4+sampl/scaley2));
            //pPainter->drawLine(QPoint(i,-m_iHeight/4), QPoint(i,-m_iHeight/4+sampr/scaley2));

            // These are decent
            //pPainter->drawLine(QPoint(i,0), QPoint(i, sampl/scaley));
            //pPainter->drawLine(QPoint(i,0), QPoint(i, -sampr/scaley));

            //m_lines[i] = QLineF(i,-sampr,i,sampl);
        } else {
            //m_lines[i] = QLineF(0,0,0,0);
        }
    }

    //pPainter->drawLines(m_lines);

    pPainter->restore();
    */
}

void GLWaveformRenderer::drawSignalPixmap(QPainter *pPainter) {

}

void GLWaveformRenderer::glDraw() {

    if(m_iWidth == 0 || m_iHeight == 0)
        return;

    if(m_pSampleBuffer == NULL) {
        fetchWaveformFromTrack();
        if(m_pSampleBuffer != NULL)
            qDebug() << "Received waveform from track";
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0,0,15,
              0,0,0,
              0,1,0);

    glDrawBuffer(GL_BACK);


    glPushMatrix();

    glScalef(1.0/5.0,m_iHeight/2, 1.0);

    drawSignalLines();

    glBegin(GL_LINES);

    glVertex3f(0,0,1.0);
    glVertex3f(0,1.0,1.0);


    glEnd();

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, m_pInternalBuffer);
    glDrawArrays(GL_LINE_LOOP,0,m_iInternalBufferSize);

    glPopMatrix();



    glFlush();
}

void GLWaveformRenderer::draw(QPainter* pPainter, QPaintEvent *pEvent) {
    pPainter->fillRect(pEvent->rect(), QBrush(bgColor));
    pPainter->setPen(signalColor);

    if(m_iWidth == 0 || m_iHeight == 0)
        return;

    //int scaley = 32767*2/ m_iHeight;
    //int scaley2 = 32767*4/ m_iHeight;
    //int sca = m_iWidth*2/ m_iHeight;


    if(m_pSampleBuffer == NULL) {
        fetchWaveformFromTrack();
        if(m_pSampleBuffer != NULL)
            qDebug() << "Received waveform from track";
    }



    //drawSignalPixmap(pPainter);



    pPainter->translate(0,m_iHeight/2);
    pPainter->scale(1.0,-1.0);
    //drawSignalLines(pPainter);


    // Draw various markers.
    pPainter->setPen(colorMarker);

    pPainter->drawLine(QLine(0,0,m_iWidth,0));
    pPainter->drawLine(QLine(m_iWidth/2,m_iHeight/2,m_iWidth/2,-m_iHeight/2));
    //pPainter->drawLine(QPoint(-h,0), QPoint(h,0));
    //pPainter->drawLine(QPoint(h,m_iHeight/2), QPoint(h,-m_iHeight/2));


}

void GLWaveformRenderer::newTrack(TrackPointer pTrack) {
    m_pTrack = pTrack;
    m_pSampleBuffer = 0;
    m_iNumSamples = 0;

}

int GLWaveformRenderer::getDesiredSecondsToDisplay() {
    return m_iDesiredSecondsToDisplay;
}

void GLWaveformRenderer::setDesiredSecondsToDisplay(int secondsToDisplay) {
    m_iDesiredSecondsToDisplay = secondsToDisplay;
}
