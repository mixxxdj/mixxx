#include <qgl.h>

#include "glwaveformrendererrgb.h"
#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

#define MAX3(a, b, c)  ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))

GLWaveformRendererRGB::GLWaveformRendererRGB(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase(waveformWidgetRenderer) {

}

GLWaveformRendererRGB::~GLWaveformRendererRGB() {

}

void GLWaveformRendererRGB::onSetup(const QDomNode& /*node*/) {

}

void GLWaveformRendererRGB::draw(QPainter* painter, QPaintEvent* /*event*/) {

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

    const int firstIndex = int(firstVisualIndex + 0.5);
    firstVisualIndex = firstIndex - firstIndex % 2;

    const int lastIndex = int(lastVisualIndex + 0.5);
    lastVisualIndex = lastIndex + lastIndex % 2;

    // Reset device for native painting
    painter->beginNativePainting();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    const double visualGain = factory->getVisualGain(::WaveformWidgetFactory::All);

    unsigned char low  = 0;
    unsigned char mid  = 0;
    unsigned char high = 0;
    unsigned char allA = 0;
    unsigned char allB = 0;

    if (m_alignment == Qt::AlignCenter) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(firstVisualIndex, lastVisualIndex, -255.0, 255.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glScalef(1.0f, visualGain * m_waveformRenderer->getGain(), 1.0f);

        glLineWidth(1.2);
        glDisable(GL_LINE_SMOOTH);

        // Draw reference line
        glBegin(GL_LINES); {
            glColor4f(m_axesColor.redF(), m_axesColor.greenF(), m_axesColor.blueF(), m_axesColor.alphaF());
            glVertex2f(firstVisualIndex, 0);
            glVertex2f(lastVisualIndex,  0);
        }
        glEnd();

        glLineWidth(1.2);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {
            for( int visualIndex = firstVisualIndex;
                 visualIndex < lastVisualIndex;
                 visualIndex += 2) {

                if( visualIndex < 0)
                    continue;

                if( visualIndex > dataSize - 1)
                    break;

                low  = math_max(data[visualIndex].filtered.low,  data[visualIndex+1].filtered.low);
                mid  = math_max(data[visualIndex].filtered.mid,  data[visualIndex+1].filtered.mid);
                high = math_max(data[visualIndex].filtered.high, data[visualIndex+1].filtered.high);
                allA = data[visualIndex].filtered.all;
                allB = data[visualIndex+1].filtered.all;

                float max = (float) MAX3(low, mid, high);
                if (max > 0.0f) {  // Prevent division by zero
                    glColor4f(low / max, mid / max, high / max, 0.9f);
                    glVertex2f(visualIndex, allA);
                    glVertex2f(visualIndex, -1.0f * allB);
                }
            }
        }

        glEnd();

    } else {  // top || bottom
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        if( m_alignment == Qt::AlignBottom)
            glOrtho(firstVisualIndex, lastVisualIndex, 0.0, 255.0, -10.0, 10.0);
        else
            glOrtho(firstVisualIndex, lastVisualIndex, 255.0, 0.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glScalef(1.0f, visualGain * m_waveformRenderer->getGain(), 1.0f);

        glLineWidth(1.2);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {
            for( int visualIndex = firstVisualIndex;
                 visualIndex < lastVisualIndex;
                 visualIndex += 2) {

                if( visualIndex < 0)
                    continue;

                if( visualIndex > dataSize - 1)
                    break;

                low  = math_max(data[visualIndex].filtered.low,  data[visualIndex+1].filtered.low);
                mid  = math_max(data[visualIndex].filtered.mid,  data[visualIndex+1].filtered.mid);
                high = math_max(data[visualIndex].filtered.high, data[visualIndex+1].filtered.high);
                allA = data[visualIndex].filtered.all;
                allB = data[visualIndex+1].filtered.all;

                float max = (float) MAX3(low, mid, high);
                if (max > 0.0f) {  // Prevent division by zero
                    glColor4f(low / max, mid / max, high / max, 0.9f);
                    glVertex2f(float(visualIndex), 0.0f);
                    glVertex2f(float(visualIndex), math_max(allA, allB));
                }
            }
        }

        glEnd();
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    painter->endNativePainting();
}
