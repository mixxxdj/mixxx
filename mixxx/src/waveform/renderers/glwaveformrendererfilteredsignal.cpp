#include <QDomNode>
#include <QLineF>
#include <QLinearGradient>
#include <QMutexLocker>

#include "controlobject.h"
#include "defs.h"
#include "glwaveformrendererfilteredsignal.h"
#include "trackinfoobject.h"
#include "waveform/waveform.h"
#include "waveformwidgetrenderer.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"
#include "waveform/waveformwidgetfactory.h"

#include <qgl.h>

GLWaveformRendererFilteredSignal::GLWaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererAbstract(waveformWidgetRenderer) {
    m_lowFilterControlObject = NULL;
    m_midFilterControlObject = NULL;
    m_highFilterControlObject = NULL;
    m_lowKillControlObject = NULL;
    m_midKillControlObject = NULL;
    m_highKillControlObject = NULL;
    m_alignment = Qt::AlignCenter;
}

GLWaveformRendererFilteredSignal::~GLWaveformRendererFilteredSignal() {
}

void GLWaveformRendererFilteredSignal::init() {
    //create controls
    m_lowFilterControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterLow"));
    m_midFilterControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterMid"));
    m_highFilterControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterHigh"));
    m_lowKillControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterLowKill"));
    m_midKillControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterMidKill"));
    m_highKillControlObject = ControlObject::getControl(ConfigKey(m_waveformRenderer->getGroup(),"filterHighKill"));
}

void GLWaveformRendererFilteredSignal::setup(const QDomNode& node) {
    QString alignString = WWidget::selectNodeQString(node, "Align");
    if (alignString == "bottom") {
        m_alignment = Qt::AlignBottom;
    } else if (alignString == "top") {
        m_alignment = Qt::AlignTop;
    } else {
        m_alignment = Qt::AlignCenter;
    }

    m_colors.setup(node);

    QColor low = m_colors.getLowColor();
    QColor mid = m_colors.getMidColor();
    QColor high = m_colors.getHighColor();

    QColor lowCenter = low;
    QColor midCenter = mid;
    QColor highCenter = high;

    low.setAlphaF(0.9);
    mid.setAlphaF(0.9);
    high.setAlphaF(0.9);

    lowCenter.setAlphaF(0.5);
    midCenter.setAlphaF(0.5);
    highCenter.setAlphaF(0.5);

    QLinearGradient gradientLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientLow.setColorAt(0.0, low);
    gradientLow.setColorAt(0.25,low.lighter(85));
    gradientLow.setColorAt(0.5, lowCenter.darker(115));
    gradientLow.setColorAt(0.75,low.lighter(85));
    gradientLow.setColorAt(1.0, low);
    m_lowBrush = QBrush(gradientLow);

    QLinearGradient gradientMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientMid.setColorAt(0.0, mid);
    gradientMid.setColorAt(0.35,mid.lighter(85));
    gradientMid.setColorAt(0.5, midCenter.darker(115));
    gradientMid.setColorAt(0.65,mid.lighter(85));
    gradientMid.setColorAt(1.0, mid);
    m_midBrush = QBrush(gradientMid);

    QLinearGradient gradientHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientHigh.setColorAt(0.0, high);
    gradientHigh.setColorAt(0.45,high.lighter(85));
    gradientHigh.setColorAt(0.5, highCenter.darker(115));
    gradientHigh.setColorAt(0.55,high.lighter(85));
    gradientHigh.setColorAt(1.0, high);
    m_highBrush = QBrush(gradientHigh);

    low.setAlphaF(0.3);
    mid.setAlphaF(0.3);
    high.setAlphaF(0.3);

    QLinearGradient gradientKilledLow(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledLow.setColorAt(0.0,low.darker(80));
    gradientKilledLow.setColorAt(0.5,lowCenter.darker(150));
    gradientKilledLow.setColorAt(1.0,low.darker(80));
    m_lowKilledBrush = QBrush(gradientKilledLow);

    QLinearGradient gradientKilledMid(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledMid.setColorAt(0.0,mid.darker(80));
    gradientKilledMid.setColorAt(0.5,midCenter.darker(150));
    gradientKilledMid.setColorAt(1.0,mid.darker(80));
    m_midKilledBrush = QBrush(gradientKilledMid);

    QLinearGradient gradientKilledHigh(QPointF(0.0,-255.0/2.0),QPointF(0.0,255.0/2.0));
    gradientKilledHigh.setColorAt(0.0,high.darker(80));
    gradientKilledHigh.setColorAt(0.5,highCenter.darker(150));
    gradientKilledHigh.setColorAt(1.0,high.darker(80));
    m_highKilledBrush = QBrush(gradientKilledHigh);
}

void GLWaveformRendererFilteredSignal::onResize() {
    m_polygon[0].resize(2*m_waveformRenderer->getWidth()+2);
    m_polygon[1].resize(2*m_waveformRenderer->getWidth()+2);
    m_polygon[2].resize(2*m_waveformRenderer->getWidth()+2);
}

void GLWaveformRendererFilteredSignal::draw(QPainter* painter, QPaintEvent* /*event*/) {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    const Waveform* waveform = pTrack->getWaveform();
    if (waveform == NULL) {
        return;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == NULL) {
        return;
    }

    double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize;
    double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize;

    const int zoom = m_waveformRenderer->getZoomFactor();

    const int firstIndex = int(firstVisualIndex+0.5);
    firstVisualIndex = firstIndex - firstIndex%2;

    const int lastIndex = int(lastVisualIndex+0.5);
    lastVisualIndex = lastIndex + lastIndex%2;

    // save the GL state set for QPainter
    painter->beginNativePainting();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(firstVisualIndex, lastVisualIndex, -255.0, 255.0, -10.0, 10.0);

    float visualSamplesPerPixel = float(lastVisualIndex - firstVisualIndex) / m_waveformRenderer->getWidth();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(.0f,.0f,.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(1.1);
    glEnable(GL_LINE_SMOOTH);

    glEnable(GL_MULTISAMPLE_ARB);

    //NOTE: if opengl < 1.2 we should use the following
    //glEnable(GL_POLYGON_SMOOTH);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    const QColor& l = m_colors.getLowColor();
    const QColor& m = m_colors.getMidColor();
    const QColor& h = m_colors.getHighColor();

    float maxLow[2];
    float maxMid[2];
    float maxHigh[2];

    float meanIndex;

    //rigth
    //glBegin(GL_TRIANGLES);
    glBegin(GL_LINES);

    for( int visualIndex = firstVisualIndex;
         visualIndex < lastVisualIndex;
         visualIndex += 2/**zoom*/) {

        if( visualIndex < 0)
            continue;

        if( visualIndex > dataSize - 1)
            break;

        maxLow[0] = (float)data[visualIndex].filtered.low;
        maxMid[0] = (float)data[visualIndex].filtered.mid;
        maxHigh[0] = (float)data[visualIndex].filtered.high;
        maxLow[1] = (float)data[visualIndex+1].filtered.low;
        maxMid[1] = (float)data[visualIndex+1].filtered.mid;
        maxHigh[1] = (float)data[visualIndex+1].filtered.high;

        meanIndex = visualIndex;

        /*
        for( int i = 1; i < zoom; i++) {
            visualIndex += 2;
            meanIndex += (float)visualIndex;
            maxLow[0] = math_max(maxLow[0],(float)data[visualIndex].filtered.low);
            maxMid[0] = math_max(maxMid[0],(float)data[visualIndex].filtered.mid);
            maxHigh[0] = math_max(maxMid[0],(float)data[visualIndex].filtered.high);
            maxLow[0] = math_max(maxLow[1],(float)data[visualIndex+1].filtered.low);
            maxMid[0] = math_max(maxMid[1],(float)data[visualIndex+1].filtered.mid);
            maxHigh[0] = math_max(maxMid[1],(float)data[visualIndex+1].filtered.high);
        }
        meanIndex /= (float)zoom;
        */

        glColor4f(l.redF(),l.greenF(),l.blueF(),0.8);
        //glVertex3f(meanIndex - visualSamplesPerPixel,0.0,-1.0f);
        //glVertex3f(meanIndex,maxLow,-1.0f);
        //glVertex3f(meanIndex + visualSamplesPerPixel,0.0,-1.0f);
        glVertex2f(meanIndex,maxLow[0]);
        glVertex2f(meanIndex,-maxLow[1]);

        glColor4f(m.redF(),m.greenF(),m.blueF(),0.9);
        //glVertex3f(meanIndex - visualSamplesPerPixel,0.0,0.0f);
        //glVertex3f(meanIndex,maxMid,0.0f);
        //glVertex3f(meanIndex + visualSamplesPerPixel,0.0,0.0f);
        glVertex2f(meanIndex,maxMid[0]);
        glVertex2f(meanIndex,-maxMid[1]);

        glColor4f(h.redF(),h.greenF(),h.blueF(),1.0);
        //glVertex3f(meanIndex - visualSamplesPerPixel,0.0,1.0f);
        //glVertex3f(meanIndex,maxHigh,1.0f);
        //glVertex3f(meanIndex + visualSamplesPerPixel,0.0,1.0f);
        glVertex2f(meanIndex,maxHigh[0]);
        glVertex2f(meanIndex,-maxHigh[1]);
    }

    glEnd();

    /*
    glScalef(1.0,-1.0,1.0);

    //left
    glBegin(GL_TRIANGLES);
    for( int visualIndex = firstVisualIndex + 1;
         visualIndex < lastVisualIndex + 1;
         visualIndex += 2) {

        if( visualIndex < 0)
            continue;

        if( visualIndex > dataSize - 1)
            break;

        maxLow = (float)data[visualIndex].filtered.low;
        maxMid = (float)data[visualIndex].filtered.mid;
        maxHigh = (float)data[visualIndex].filtered.high;

        for( int i = 0; i < zoom; i++) {
            visualIndex += 2;
            maxLow = math_max(maxLow,(float)data[visualIndex].filtered.low);
            maxMid = math_max(maxMid,(float)data[visualIndex].filtered.mid);
            maxHigh = math_max(maxMid,(float)data[visualIndex].filtered.high);
        }

        glColor4f(l.redF(),l.greenF(),l.blueF(),0.8);
        glVertex3f(visualIndex - visualSamplesPerPixel,0.0,-1.0f);
        glVertex3f(visualIndex,maxLow,-1.0f);
        glVertex3f(visualIndex + visualSamplesPerPixel,0.0,-1.0f);

        glColor4f(m.redF(),m.greenF(),m.blueF(),0.9);
        glVertex3f(visualIndex - visualSamplesPerPixel,0.0,0.0f);
        glVertex3f(visualIndex,maxMid,0.0f);
        glVertex3f(visualIndex + visualSamplesPerPixel,0.0,0.0f);

        glColor4f(h.redF(),h.greenF(),h.blueF(),1.0);
        glVertex3f(visualIndex - visualSamplesPerPixel,0.0,1.0f);
        glVertex3f(visualIndex,maxHigh,1.0f);
        glVertex3f(visualIndex + visualSamplesPerPixel,0.0,1.0f);
    }
    glEnd();*/

    //DEBUG
    /*glDisable(GL_ALPHA_TEST);
    glBegin(GL_LINE_LOOP);
    {
        glColor4f(0.5,1.0,0.5,0.25);
        glVertex3f(firstVisualIndex,-1.0f, 0.0f);
        glVertex3f(lastVisualIndex, 1.0f, 0.0f);
        glVertex3f(lastVisualIndex,-1.0f, 0.0f);
        glVertex3f(firstVisualIndex, 1.0f, 0.0f);
    }
    glEnd();*/

    glDisable(GL_BLEND);

    painter->endNativePainting();
}
