#include <QPainter>
#include <QGLContext>
#include <QtDebug>

#include "glwaveformwidget.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/qtwaveformrendererfilteredsignal.h"
#include "waveform/renderers/glwaveformrendererfilteredsignal.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrenderbeat.h"
#include "sharedglcontext.h"

GLWaveformWidget::GLWaveformWidget( const char* group, QWidget* parent)
        : QGLWidget(parent, SharedGLContext::getWidget()),
          WaveformWidgetAbstract(group) {

    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<GLWaveformRendererFilteredSignal>();
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoBufferSwap(false);

    qDebug() << "Created QGLWidget. Context"
             << "Valid:" << context()->isValid()
             << "Sharing:" << context()->isSharing();
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }
    m_initSuccess = init();
}

GLWaveformWidget::~GLWaveformWidget() {
}

void GLWaveformWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(static_cast<QGLWidget*>(this));
}

void GLWaveformWidget::paintEvent( QPaintEvent* event) {
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }
    QPainter painter(this);
    draw(&painter, event);
}

void GLWaveformWidget::postRender() {
    QGLWidget::swapBuffers();
}
