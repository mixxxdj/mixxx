#include "controlobjectthreadmain.h"

#include "defs.h"
#include "glwaveformrendererfilteredsignal.h"
#include "trackinfoobject.h"
#include "waveform/waveform.h"
#include "waveformwidgetrenderer.h"

#include "waveform/waveformwidgetfactory.h"

#include <QDomNode>

#include <qgl.h>

GLWaveformRendererFilteredSignal::GLWaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase(waveformWidgetRenderer) {

}

GLWaveformRendererFilteredSignal::~GLWaveformRendererFilteredSignal() {

}

void GLWaveformRendererFilteredSignal::onInit() {

}

void GLWaveformRendererFilteredSignal::onSetup(const QDomNode& /*node*/) {

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

    const int firstIndex = int(firstVisualIndex+0.5);
    firstVisualIndex = firstIndex - firstIndex%2;

    const int lastIndex = int(lastVisualIndex+0.5);
    lastVisualIndex = lastIndex + lastIndex%2;

    // save the GL state set for QPainter
    painter->beginNativePainting();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const QColor& l = m_colors.getLowColor();
    const QColor& m = m_colors.getMidColor();
    const QColor& h = m_colors.getHighColor();

    // Per-band gain from the EQ knobs.
    float lowGain(1.0), midGain(1.0), highGain(1.0);
    if (m_lowFilterControlObject &&
            m_midFilterControlObject &&
            m_highFilterControlObject) {
        lowGain = m_lowFilterControlObject->get();
        midGain = m_midFilterControlObject->get();
        highGain = m_highFilterControlObject->get();
    }

    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    double visualGain = factory->getVisualGain(::WaveformWidgetFactory::All);
    lowGain *= factory->getVisualGain(WaveformWidgetFactory::Low);
    midGain *= factory->getVisualGain(WaveformWidgetFactory::Mid);
    highGain *= factory->getVisualGain(WaveformWidgetFactory::High);

    float maxLow[2];
    float maxMid[2];
    float maxHigh[2];

    float meanIndex;

    glPushMatrix();

    if( m_alignment == Qt::AlignCenter) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(firstVisualIndex, lastVisualIndex, -255.0, 255.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glScalef(1.f,visualGain*m_waveformRenderer->getGain(),1.f);

        glLineWidth(1.0);
        glDisable(GL_LINE_SMOOTH);

        //draw reference line
        glBegin(GL_LINES); {
            glColor4f(m_axesColor.redF(),m_axesColor.greenF(),m_axesColor.blueF(),m_axesColor.alphaF());
            glVertex2f(firstVisualIndex,0);
            glVertex2f(lastVisualIndex,0);
        }
        glEnd();

        glLineWidth(1.1);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {
            for( int visualIndex = firstVisualIndex;
                 visualIndex < lastVisualIndex;
                 visualIndex += 2) {

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

                glColor4f(l.redF(),l.greenF(),l.blueF(),0.8);
                glVertex2f(meanIndex,lowGain*maxLow[0]);
                glVertex2f(meanIndex,-1.f*lowGain*maxLow[1]);

                glColor4f(m.redF(),m.greenF(),m.blueF(),0.85);
                glVertex2f(meanIndex,midGain*maxMid[0]);
                glVertex2f(meanIndex,-1.f*midGain*maxMid[1]);

                glColor4f(h.redF(),h.greenF(),h.blueF(),0.9);
                glVertex2f(meanIndex,highGain*maxHigh[0]);
                glVertex2f(meanIndex,-1.f*highGain*maxHigh[1]);
            }
        }
        glEnd();
    } else { //top || bottom
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if( m_alignment == Qt::AlignBottom)
            glOrtho(firstVisualIndex, lastVisualIndex, 0.0, 255.0, -10.0, 10.0);
        else
            glOrtho(firstVisualIndex, lastVisualIndex, 255.0, 0.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glScalef(1.f,visualGain*m_waveformRenderer->getGain(),1.f);

        glLineWidth(1.1);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {
            for( int visualIndex = firstVisualIndex;
                 visualIndex < lastVisualIndex;
                 visualIndex += 2) {

                if( visualIndex < 0)
                    continue;

                if( visualIndex > dataSize - 1)
                    break;

                maxLow[0] = (float)data[visualIndex].filtered.low;
                maxLow[1] = (float)data[visualIndex+1].filtered.low;
                maxMid[0] = (float)data[visualIndex].filtered.mid;
                maxMid[1] = (float)data[visualIndex+1].filtered.mid;
                maxHigh[0] = (float)data[visualIndex].filtered.high;
                maxHigh[1] = (float)data[visualIndex+1].filtered.high;

                glColor4f(l.redF(),l.greenF(),l.blueF(),0.8);
                glVertex2f(float(visualIndex),0.f);
                glVertex2f(float(visualIndex),lowGain*math_max(maxLow[0],maxLow[1]));

                glColor4f(m.redF(),m.greenF(),m.blueF(),0.85);
                glVertex2f(float(visualIndex),0.f);
                glVertex2f(float(visualIndex),midGain*math_max(maxMid[0],maxMid[1]));

                glColor4f(h.redF(),h.greenF(),h.blueF(),0.9);
                glVertex2f(float(visualIndex),0.f);
                glVertex2f(float(visualIndex),highGain*math_max(maxHigh[0],maxHigh[1]));
            }
        }
        glEnd();
    }

    glPopMatrix();

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
