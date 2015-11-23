#include "glwaveformrenderersimplesignal.h"

#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"

#include "waveform/waveformwidgetfactory.h"
#include "util/math.h"

#include <qgl.h>

GLWaveformRendererSimpleSignal::GLWaveformRendererSimpleSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase(waveformWidgetRenderer) {

}

GLWaveformRendererSimpleSignal::~GLWaveformRendererSimpleSignal() {
}

void GLWaveformRendererSimpleSignal::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

inline void setPoint(QPointF& point, qreal x, qreal y) {
    point.setX(x);
    point.setY(y);
}

void GLWaveformRendererSimpleSignal::draw(QPainter* painter, QPaintEvent* /*event*/) {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    ConstWaveformPointer waveform = pTrack->getWaveform();
    if (waveform.isNull()) {
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

    // Reset device for native painting
    painter->beginNativePainting();

#ifndef __OPENGLES__

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float allGain(1.0);
    getGains(&allGain, NULL, NULL, NULL);

    float maxAll[2];

    if (m_alignment == Qt::AlignCenter) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(firstVisualIndex, lastVisualIndex, -255.0, 255.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glScalef(1.f, allGain, 1.f);

        glLineWidth(1.0);
        glDisable(GL_LINE_SMOOTH);

        //draw reference line
        glBegin(GL_LINES); {
            glColor4f(m_axesColor_r, m_axesColor_g,
                      m_axesColor_b, m_axesColor_a);
            glVertex2f(firstVisualIndex,0);
            glVertex2f(lastVisualIndex,0);
        }
        glEnd();

        glLineWidth(1.1);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {
            for (int visualIndex = firstVisualIndex;
                 visualIndex < lastVisualIndex;
                 visualIndex += 2) {

                if (visualIndex < 0)
                    continue;

                if (visualIndex > dataSize - 1)
                    break;

                maxAll[0] = (float)data[visualIndex].filtered.all;
                maxAll[1] = (float)data[visualIndex+1].filtered.all;
                glColor4f(m_signalColor_r, m_signalColor_g, m_signalColor_b, 0.9);
                glVertex2f(visualIndex,maxAll[0]);
                glVertex2f(visualIndex,-1.f*maxAll[1]);
            }
        }
        glEnd();
    } else { //top || bottom
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        if (m_alignment == Qt::AlignBottom)
            glOrtho(firstVisualIndex, lastVisualIndex, 0.0, 255.0, -10.0, 10.0);
        else
            glOrtho(firstVisualIndex, lastVisualIndex, 255.0, 0.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glScalef(1.f, allGain, 1.f);

        glLineWidth(1.1);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {
            for (int visualIndex = firstVisualIndex;
                 visualIndex < lastVisualIndex;
                 visualIndex += 2) {

                if (visualIndex < 0)
                    continue;

                if (visualIndex > dataSize - 1)
                    break;

                maxAll[0] = (float)data[visualIndex].filtered.all;
                maxAll[1] = (float)data[visualIndex+1].filtered.all;
                glColor4f(m_signalColor_r, m_signalColor_g, m_signalColor_b, 0.8);
                glVertex2f(float(visualIndex),0.f);
                glVertex2f(float(visualIndex),math_max(maxAll[0],maxAll[1]));
            }
        }
        glEnd();
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

#endif

    painter->endNativePainting();
}
