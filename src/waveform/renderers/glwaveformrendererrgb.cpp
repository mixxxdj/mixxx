#include <qgl.h>

#include "glwaveformrendererrgb.h"
#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "controlobjectthread.h"
#include "util/math.h"

GLWaveformRendererRGB::GLWaveformRendererRGB(
        WaveformWidgetRenderer* waveformWidgetRenderer)
    : WaveformRendererSignalBase(waveformWidgetRenderer) {

}

GLWaveformRendererRGB::~GLWaveformRendererRGB() {

}

void GLWaveformRendererRGB::onSetup(const QDomNode& /* node */) {

}

void GLWaveformRendererRGB::setup(const QDomNode& node,
                                       const SkinContext& context) {

    WaveformRendererSignalBase::setup(node, context);

    m_lowColor.setNamedColor(context.selectString(node, "SignalRGBLowColor"));
    if (!m_lowColor.isValid()) {
        m_lowColor = Qt::red;
    }
    m_lowColor  = WSkinColor::getCorrectColor(m_lowColor);
    m_lowColor.getRgbF(&m_lowColor_r, &m_lowColor_g, &m_lowColor_b);

    m_midColor.setNamedColor(context.selectString(node, "SignalRGBMidColor"));
    if (!m_midColor.isValid()) {
        m_midColor = Qt::green;
    }
    m_midColor  = WSkinColor::getCorrectColor(m_midColor);
    m_midColor.getRgbF(&m_midColor_r, &m_midColor_g, &m_midColor_b);

    m_highColor.setNamedColor(context.selectString(node, "SignalRGBHighColor"));
    if (!m_highColor.isValid()) {
        m_highColor = Qt::blue;
    }
    m_highColor = WSkinColor::getCorrectColor(m_highColor);
    m_highColor.getRgbF(&m_highColor_r, &m_highColor_g, &m_highColor_b);
}

void GLWaveformRendererRGB::draw(QPainter* painter, QPaintEvent* /*event*/) {

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

    const int firstIndex = int(firstVisualIndex + 0.5);
    firstVisualIndex = firstIndex - firstIndex % 2;

    const int lastIndex = int(lastVisualIndex + 0.5);
    lastVisualIndex = lastIndex + lastIndex % 2;

    // Reset device for native painting
    painter->beginNativePainting();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Per-band gain from the EQ knobs.
    float allGain(1.0), lowGain(1.0), midGain(1.0), highGain(1.0);
    getGains(&allGain, &lowGain, &midGain, &highGain);

    if (m_alignment == Qt::AlignCenter) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(firstVisualIndex, lastVisualIndex, -255.0, 255.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glScalef(1.0f, allGain, 1.0f);

        glLineWidth(1.2);
        glDisable(GL_LINE_SMOOTH);

        // Draw reference line
        glBegin(GL_LINES); {
            glColor4f(m_axesColor_r, m_axesColor_g, m_axesColor_b, m_axesColor_a);
            glVertex2f(firstVisualIndex, 0);
            glVertex2f(lastVisualIndex,  0);
        }
        glEnd();

        glLineWidth(2.0);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {
            for (int visualIndex = firstVisualIndex;
                 visualIndex < lastVisualIndex;
                 visualIndex += 2) {

                if (visualIndex < 0) {
                    continue;
                }

                if (visualIndex > dataSize - 1) {
                    break;
                }

                float left_low    = lowGain  * (float) data[visualIndex].filtered.low;
                float left_mid    = midGain  * (float) data[visualIndex].filtered.mid;
                float left_high   = highGain * (float) data[visualIndex].filtered.high;
                float left_all    = sqrtf(left_low * left_low + left_mid * left_mid + left_high * left_high);
                float left_red    = left_low  * m_lowColor_r + left_mid  * m_midColor_r + left_high  * m_highColor_r;
                float left_green  = left_low  * m_lowColor_g + left_mid  * m_midColor_g + left_high  * m_highColor_g;
                float left_blue   = left_low  * m_lowColor_b + left_mid  * m_midColor_b + left_high  * m_highColor_b;
                float left_max    = math_max3(left_red, left_green, left_blue);
                if (left_max > 0.0f) {  // Prevent division by zero
                    glColor4f(left_red / left_max, left_green / left_max, left_blue / left_max, 0.8f);
                    glVertex2f(visualIndex, 0.0f);
                    glVertex2f(visualIndex, left_all);
                }

                float right_low   = lowGain  * (float) data[visualIndex+1].filtered.low;
                float right_mid   = midGain  * (float) data[visualIndex+1].filtered.mid;
                float right_high  = highGain * (float) data[visualIndex+1].filtered.high;
                float right_all   = sqrtf(right_low * right_low + right_mid * right_mid + right_high * right_high);
                float right_red   = right_low * m_lowColor_r + right_mid * m_midColor_r + right_high * m_highColor_r;
                float right_green = right_low * m_lowColor_g + right_mid * m_midColor_g + right_high * m_highColor_g;
                float right_blue  = right_low * m_lowColor_b + right_mid * m_midColor_b + right_high * m_highColor_b;
                float right_max   = math_max3(right_red, right_green, right_blue);
                if (right_max > 0.0f) {  // Prevent division by zero
                    glColor4f(right_red / right_max, right_green / right_max, right_blue / right_max, 0.8f);
                    glVertex2f(visualIndex, 0.0f);
                    glVertex2f(visualIndex, -1.0f * right_all);
                }
            }
        }

        glEnd();

    } else {  // top || bottom
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        if (m_alignment == Qt::AlignBottom) {
            glOrtho(firstVisualIndex, lastVisualIndex, 0.0, 255.0, -10.0, 10.0);
        } else {
            glOrtho(firstVisualIndex, lastVisualIndex, 255.0, 0.0, -10.0, 10.0);
        }

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glScalef(1.0f, allGain, 1.0f);

        glLineWidth(2.0);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {
            for (int visualIndex = firstVisualIndex;
                 visualIndex < lastVisualIndex;
                 visualIndex += 2) {

                if (visualIndex < 0) {
                    continue;
                }

                if (visualIndex > dataSize - 1) {
                    break;
                }

                float low  = lowGain  * (float) math_max(data[visualIndex].filtered.low,  data[visualIndex+1].filtered.low);
                float mid  = midGain  * (float) math_max(data[visualIndex].filtered.mid,  data[visualIndex+1].filtered.mid);
                float high = highGain * (float) math_max(data[visualIndex].filtered.high, data[visualIndex+1].filtered.high);

                float all = sqrtf(low * low + mid * mid + high * high);

                float red   = low * m_lowColor_r + mid * m_midColor_r + high * m_highColor_r;
                float green = low * m_lowColor_g + mid * m_midColor_g + high * m_highColor_g;
                float blue  = low * m_lowColor_b + mid * m_midColor_b + high * m_highColor_b;

                float max = math_max3(red, green, blue);
                if (max > 0.0f) {  // Prevent division by zero
                    glColor4f(red / max, green / max, blue / max, 0.9f);
                    glVertex2f(float(visualIndex), 0.0f);
                    glVertex2f(float(visualIndex), all);
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
