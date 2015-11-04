#include <qgl.h>

#include "glwaveformrendererrgb.h"
#include "waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "controlobjectthread.h"

#define MAX3(a, b, c)  ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))

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

    m_lowColor.setNamedColor(context.selectString(node, "SignalLowColor"));
    if (!m_lowColor.isValid()) {
        m_lowColor = Qt::red;
    }
    m_lowColor  = WSkinColor::getCorrectColor(m_lowColor);

    m_midColor.setNamedColor(context.selectString(node, "SignalMidColor"));
    if (!m_midColor.isValid()) {
        m_midColor = Qt::green;
    }
    m_midColor  = WSkinColor::getCorrectColor(m_midColor);

    m_highColor.setNamedColor(context.selectString(node, "SignalHighColor"));
    if (!m_highColor.isValid()) {
        m_highColor = Qt::blue;
    }
    m_highColor = WSkinColor::getCorrectColor(m_highColor);
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

    // Per-band gain from the EQ knobs.
    float lowGain(1.0), midGain(1.0), highGain(1.0);
    if (m_pLowFilterControlObject && m_pMidFilterControlObject && m_pHighFilterControlObject) {
        lowGain = m_pLowFilterControlObject->get();
        midGain = m_pMidFilterControlObject->get();
        highGain = m_pHighFilterControlObject->get();
    }

    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    const double visualGain = factory->getVisualGain(::WaveformWidgetFactory::All);
    lowGain  *= factory->getVisualGain(WaveformWidgetFactory::Low);
    midGain  *= factory->getVisualGain(WaveformWidgetFactory::Mid);
    highGain *= factory->getVisualGain(WaveformWidgetFactory::High);

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
            glColor4f(m_axesColor_r, m_axesColor_g, m_axesColor_b, m_axesColor_a);
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

                float left_low    = lowGain  * (float) data[visualIndex].filtered.low;
                float left_mid    = midGain  * (float) data[visualIndex].filtered.mid;
                float left_high   = highGain * (float) data[visualIndex].filtered.high;
                float left_all    = sqrtf(left_low * left_low + left_mid * left_mid + left_high * left_high);
                float left_red    = left_low  * m_lowColor.red()   + left_mid  * m_midColor.red()   + left_high  * m_highColor.red();
                float left_green  = left_low  * m_lowColor.green() + left_mid  * m_midColor.green() + left_high  * m_highColor.green();
                float left_blue   = left_low  * m_lowColor.blue()  + left_mid  * m_midColor.blue()  + left_high  * m_highColor.blue();
                float left_max    = MAX3(left_red, left_green, left_blue);
                if (left_max > 0.0f) {  // Prevent division by zero
                    glColor4f(left_red / left_max, left_green / left_max, left_blue / left_max, 0.8f);
                    glVertex2f(visualIndex, 0.0f);
                    glVertex2f(visualIndex, left_all);
                }

                float right_low   = lowGain  * (float) data[visualIndex+1].filtered.low;
                float right_mid   = midGain  * (float) data[visualIndex+1].filtered.mid;
                float right_high  = highGain * (float) data[visualIndex+1].filtered.high;
                float right_all   = sqrtf(right_low * right_low + right_mid * right_mid + right_high * right_high);
                float right_red   = right_low * m_lowColor.red()   + right_mid * m_midColor.red()   + right_high * m_highColor.red();
                float right_green = right_low * m_lowColor.green() + right_mid * m_midColor.green() + right_high * m_highColor.green();
                float right_blue  = right_low * m_lowColor.blue()  + right_mid * m_midColor.blue()  + right_high * m_highColor.blue();
                float right_max   = MAX3(right_red, right_green, right_blue);
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

                float low  = lowGain  * (float) math_max(data[visualIndex].filtered.low,  data[visualIndex+1].filtered.low);
                float mid  = midGain  * (float) math_max(data[visualIndex].filtered.mid,  data[visualIndex+1].filtered.mid);
                float high = highGain * (float) math_max(data[visualIndex].filtered.high, data[visualIndex+1].filtered.high);

                float all = sqrtf(low * low + mid * mid + high * high);

                float red   = low * m_lowColor.red()   + mid * m_midColor.red()   + high * m_highColor.red();
                float green = low * m_lowColor.green() + mid * m_midColor.green() + high * m_highColor.green();
                float blue  = low * m_lowColor.blue()  + mid * m_midColor.blue()  + high * m_highColor.blue();

                float max = MAX3(red, green, blue);
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
