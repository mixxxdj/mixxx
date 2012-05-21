#include "qtwaveformwidget.h"

#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/renderers/waveformrenderbackground.h"
#include "waveform/renderers/qtwaveformrendererfilteredsignal.h"
#include "waveform/renderers/waveformrendererpreroll.h"
#include "waveform/renderers/waveformrendermark.h"
#include "waveform/renderers/waveformrendermarkrange.h"
#include "waveform/renderers/waveformrendererendoftrack.h"
#include "waveform/renderers/waveformrenderbeat.h"
#include "sharedglcontext.h"

#include <QPainter>
#include <QGLContext>

QtWaveformWidget::QtWaveformWidget( const char* group, QWidget* parent) :
        QGLWidget(SharedGLContext::getContext(), parent),
        WaveformWidgetAbstract(group) {

    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    addRenderer<QtWaveformRendererFilteredSignal>();
    addRenderer<WaveformRenderMark>();
    addRenderer<WaveformRenderBeat>();

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);

    setAutoBufferSwap(false);

    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }
    init();
}

QtWaveformWidget::~QtWaveformWidget() {
}

void QtWaveformWidget::castToQWidget() {
    m_widget = static_cast<QWidget*>(static_cast<QGLWidget*>(this));
}

void QtWaveformWidget::paintEvent( QPaintEvent* event) {
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }
    QPainter painter(this);
    draw(&painter, event);
}

void QtWaveformWidget::postRender() {
    QGLWidget::swapBuffers();
}
