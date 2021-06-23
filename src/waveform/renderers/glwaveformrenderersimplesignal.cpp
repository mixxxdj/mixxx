#include "waveform/renderers/glwaveformrenderersimplesignal.h"
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include "track/track.h"
#include "util/math.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveformwidgetrenderer.h"

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
    maybeInitializeGL();

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
    if (data == nullptr) {
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

    float allGain(1.0);
    getGains(&allGain, nullptr, nullptr, nullptr);

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

        glScalef(1.f, allGain, 1.f);

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

            glColor4f(static_cast<GLfloat>(m_signalColor_r),
                    static_cast<GLfloat>(m_signalColor_g),
                    static_cast<GLfloat>(m_signalColor_b),
                    0.9f);
            for (int visualIndex = firstIndex;
                    visualIndex < lastIndex;
                    visualIndex += 2) {

                GLfloat maxAll0 = data[visualIndex].filtered.all;
                GLfloat maxAll1 = data[visualIndex+1].filtered.all;
                glVertex2f(visualIndex, maxAll0);
                glVertex2f(visualIndex, -1.f * maxAll1);
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
        if (m_alignment == Qt::AlignBottom || m_alignment == Qt::AlignRight) {
            glOrtho(firstVisualIndex, lastVisualIndex, 0.0, 255.0, -10.0, 10.0);
        } else {
            glOrtho(firstVisualIndex, lastVisualIndex, 255.0, 0.0, -10.0, 10.0);
        }

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glScalef(1.f, allGain, 1.f);

        glLineWidth(lineWidth);
        glEnable(GL_LINE_SMOOTH);

        glBegin(GL_LINES); {
            int firstIndex = math_max(static_cast<int>(firstVisualIndex), 0);
            int lastIndex = math_min(static_cast<int>(lastVisualIndex), dataSize);

            glColor4f(static_cast<GLfloat>(m_signalColor_r),
                    static_cast<GLfloat>(m_signalColor_g),
                    static_cast<GLfloat>(m_signalColor_b),
                    0.8f);
            for (int visualIndex = firstIndex;
                    visualIndex < lastIndex;
                    visualIndex += 2) {

                GLfloat maxAll = math_max(
                        data[visualIndex].filtered.all,
                        data[visualIndex+1].filtered.all);
                glVertex2f(float(visualIndex), 0.f);
                glVertex2f(float(visualIndex), maxAll);
            }
        }
        glEnd();
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    painter->endNativePainting();
}

#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
