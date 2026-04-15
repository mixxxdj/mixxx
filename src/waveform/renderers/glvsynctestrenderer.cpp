#include "waveform/renderers/glvsynctestrenderer.h"
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include "util/performancetimer.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

GLVSyncTestRenderer::GLVSyncTestRenderer(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : GLWaveformRendererSignal(waveformWidgetRenderer,
                  WaveformRendererSignalBase::Option::None),
          m_drawcount(0) {
}

GLVSyncTestRenderer::~GLVSyncTestRenderer() {
}

void GLVSyncTestRenderer::onSetup(const QDomNode &node) {
    Q_UNUSED(node);
}

inline void setPoint(QPointF& point, qreal x, qreal y) {
    point.setX(x);
    point.setY(y);
}

void GLVSyncTestRenderer::draw(QPainter* painter, QPaintEvent* /*event*/) {
    PerformanceTimer timer;
    //mixxx::Duration t5, t6, t7, t8, t9, t10, t11, t12, t13;

    timer.start();

    ConstWaveformPointer pWaveform = m_waveformRenderer->getWaveform();
    if (pWaveform.isNull()) {
        return;
    }

    const double audioVisualRatio = pWaveform->getAudioVisualRatio();
    if (audioVisualRatio <= 0) {
        return;
    }

    const int dataSize = pWaveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = pWaveform->data();
    if (data == nullptr) {
        return;
    }

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() *
            trackSamples / audioVisualRatio;
    double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() *
            trackSamples / audioVisualRatio;

    const int firstIndex = int(firstVisualIndex + 0.5);
    firstVisualIndex = firstIndex - firstIndex % 2;

    const int lastIndex = int(lastVisualIndex + 0.5);
    lastVisualIndex = lastIndex + lastIndex % 2;

    //t5 = timer.restart(); // 910 ns

    // Reset device for native painting
    painter->beginNativePainting();

    //t6 = timer.restart(); // 29,150 ns

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //t7 = timer.restart(); // 5,770 ns

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    if (m_orientation == Qt::Vertical) {
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        glScalef(-1.0f, 1.0f, 1.0f);
    }

    //t8 = timer.restart(); // 2,611 ns

    if (m_alignment == Qt::AlignCenter) {
        glOrtho(firstVisualIndex, lastVisualIndex, -255.0, 255.0, -10.0, 10.0);
    } else if (m_alignment == Qt::AlignBottom) {
        glOrtho(firstVisualIndex, lastVisualIndex, 0.0, 255.0, -10.0, 10.0);
    } else {
        glOrtho(firstVisualIndex, lastVisualIndex, 255.0, 0.0, -10.0, 10.0);
    }

    //t9 = timer.restart(); // 1,320 ns

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    //t10 = timer.restart(); // 915 ns

    if (++m_drawcount & 1) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glRectf(0, 0, 1, 1);

    //t11 = timer.restart(); // 217,985 ns

    glEnd();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    //t12 = timer.restart(); // 22,426 ns
    painter->endNativePainting();

    //t13 = timer.restart(); // 1,430 ns

    //qDebug() << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13;

    //qDebug() << timer.restart(); // 129,498 ns
}

#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
