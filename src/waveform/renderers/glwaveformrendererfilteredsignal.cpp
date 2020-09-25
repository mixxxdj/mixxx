#include "glwaveformrendererfilteredsignal.h"
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include <QDomNode>

#include "track/track.h"
#include "waveform/waveform.h"
#include "waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"
#include "util/math.h"

GLWaveformRendererFilteredSignal::GLWaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererSignalBase(waveformWidgetRenderer) {
    initializeOpenGLFunctions();
}

GLWaveformRendererFilteredSignal::~GLWaveformRendererFilteredSignal() {

}

void GLWaveformRendererFilteredSignal::onSetup(const QDomNode& /*node*/) {

}

void GLWaveformRendererFilteredSignal::draw(QPainter* painter, QPaintEvent* /*event*/) {

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

    auto firstVisualIndex = static_cast<GLfloat>(
            m_waveformRenderer->getFirstDisplayedPosition() * dataSize);
    auto lastVisualIndex = static_cast<GLfloat>(
            m_waveformRenderer->getLastDisplayedPosition() * dataSize);
    const auto lineWidth = static_cast<GLfloat>(
            1.0 / m_waveformRenderer->getVisualSamplePerPixel() + 1);

    const auto firstIndex = static_cast<int>(firstVisualIndex + 0.5);
    firstVisualIndex = firstIndex - firstIndex%2;

    const auto lastIndex = static_cast<int>(lastVisualIndex + 0.5);
    lastVisualIndex = lastIndex + lastIndex%2;

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
        if (m_orientation == Qt::Vertical) {
            glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
            glScalef(-1.0f, 1.0f, 1.0f);
        }
        glOrtho(firstVisualIndex, lastVisualIndex, -255.0, 255.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glScalef(1.f,allGain,1.f);

        glLineWidth(1.0);
        glDisable(GL_LINE_SMOOTH);

        //draw reference line
        glBegin(GL_LINES); {
            glColor4f(static_cast<GLfloat>(m_axesColor_r),
                    static_cast<GLfloat>(m_axesColor_g),
                    static_cast<GLfloat>(m_axesColor_b),
                    static_cast<GLfloat>(m_axesColor_a));
            glVertex2f(firstVisualIndex,0);
            glVertex2f(lastVisualIndex,0);
        }
        glEnd();

        glLineWidth(lineWidth);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {

            int firstIndex = math_max(static_cast<int>(firstVisualIndex), 0);
            int lastIndex = math_min(static_cast<int>(lastVisualIndex), dataSize);

            glColor4f(static_cast<GLfloat>(m_lowColor_r),
                    static_cast<GLfloat>(m_lowColor_g),
                    static_cast<GLfloat>(m_lowColor_b),
                    0.8f);
            for (int visualIndex = firstIndex;
                    visualIndex < lastIndex;
                    visualIndex += 2) {

                GLfloat maxLow0 = data[visualIndex].filtered.low;
                GLfloat maxLow1 = data[visualIndex+1].filtered.low;

                glVertex2f(visualIndex,lowGain*maxLow0);
                glVertex2f(visualIndex,-1.f*lowGain*maxLow1);
            }

            glColor4f(static_cast<GLfloat>(m_midColor_r),
                    static_cast<GLfloat>(m_midColor_g),
                    static_cast<GLfloat>(m_midColor_b),
                    0.85f);
            for (int visualIndex = firstIndex;
                    visualIndex < lastIndex;
                    visualIndex += 2) {

                GLfloat maxMid0 = data[visualIndex].filtered.mid;
                GLfloat maxMid1 = data[visualIndex+1].filtered.mid;

                glVertex2f(visualIndex, midGain * maxMid0);
                glVertex2f(visualIndex,-1.f * midGain * maxMid1);
            }

            glColor4f(static_cast<GLfloat>(m_highColor_r),
                    static_cast<GLfloat>(m_highColor_g),
                    static_cast<GLfloat>(m_highColor_b),
                    0.9f);
            for (int visualIndex = firstIndex;
                    visualIndex < lastIndex;
                    visualIndex += 2) {

                GLfloat maxHigh0 = data[visualIndex].filtered.high;
                GLfloat maxHigh1 = data[visualIndex + 1].filtered.high;

                glVertex2f(visualIndex, highGain * maxHigh0);
                glVertex2f(visualIndex, -1.f * highGain * maxHigh1);
            }
        }
        glEnd();
    } else { //top || bottom
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        if (m_orientation == Qt::Vertical) {
            glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
            glScalef(-1.0f, 1.0f, 1.0f);
        }
        if (m_alignment == Qt::AlignBottom || m_alignment == Qt::AlignRight)
            glOrtho(firstVisualIndex, lastVisualIndex, 0.0, 255.0, -10.0, 10.0);
        else
            glOrtho(firstVisualIndex, lastVisualIndex, 255.0, 0.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glScalef(1.f,allGain,1.f);

        glLineWidth(lineWidth);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {

            int firstIndex = math_max(static_cast<int>(firstVisualIndex), 0);
            int lastIndex = math_min(static_cast<int>(lastVisualIndex), dataSize);

            glColor4f(static_cast<GLfloat>(m_lowColor_r),
                    static_cast<GLfloat>(m_lowColor_g),
                    static_cast<GLfloat>(m_lowColor_b),
                    0.8f);
            for (int visualIndex = firstIndex;
                    visualIndex < lastIndex;
                    visualIndex += 2) {

                GLfloat maxLow = math_max(
                        data[visualIndex].filtered.low,
                        data[visualIndex+1].filtered.low);

                glVertex2f(visualIndex, 0);
                glVertex2f(visualIndex, lowGain * maxLow);
            }

            glColor4f(static_cast<GLfloat>(m_midColor_r),
                    static_cast<GLfloat>(m_midColor_g),
                    static_cast<GLfloat>(m_midColor_b),
                    0.85f);
            for (int visualIndex = firstIndex;
                    visualIndex < lastIndex;
                    visualIndex += 2) {

                GLfloat maxMid = math_max(
                        data[visualIndex].filtered.mid,
                        data[visualIndex+1].filtered.mid);

                glVertex2f(visualIndex, 0.f);
                glVertex2f(visualIndex, midGain * maxMid);
            }

            glColor4f(static_cast<GLfloat>(m_highColor_r),
                    static_cast<GLfloat>(m_highColor_g),
                    static_cast<GLfloat>(m_highColor_b),
                    0.9f);
            for (int visualIndex = firstIndex;
                    visualIndex < lastIndex;
                    visualIndex += 2) {

                GLfloat maxHigh = math_max(
                        data[visualIndex].filtered.high,
                        data[visualIndex + 1].filtered.high);

                glVertex2f(visualIndex, 0.f);
                glVertex2f(visualIndex, highGain * maxHigh);
            }
        }
        glEnd();
    }

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

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    painter->endNativePainting();
}

#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
