
#include "waveform/widgets/allshader/waveformwidget.h"

#include <QApplication>
#include <QWheelEvent>

#include "waveform/renderers/allshader/waveformrenderbackground.h"
#include "waveform/renderers/allshader/waveformrenderbeat.h"
#include "waveform/renderers/allshader/waveformrendererabstract.h"
#include "waveform/renderers/allshader/waveformrendererendoftrack.h"
#include "waveform/renderers/allshader/waveformrendererfiltered.h"
#include "waveform/renderers/allshader/waveformrendererhsv.h"
#include "waveform/renderers/allshader/waveformrendererpreroll.h"
#include "waveform/renderers/allshader/waveformrendererrgb.h"
#include "waveform/renderers/allshader/waveformrenderersimple.h"
#include "waveform/renderers/allshader/waveformrendererslipmode.h"
#include "waveform/renderers/allshader/waveformrenderertextured.h"
#include "waveform/renderers/allshader/waveformrendermark.h"
#include "waveform/renderers/allshader/waveformrendermarkrange.h"
#include "waveform/widgets/allshader/moc_waveformwidget.cpp"

namespace allshader {

WaveformWidget::WaveformWidget(QWidget* parent,
        WaveformWidgetType::Type type,
        const QString& group,
        WaveformRendererSignalBase::Options options)
        : WGLWidget(parent), WaveformWidgetAbstract(group) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();

    allshader::WaveformRendererSignalBase* waveformSignalRenderer =
            addWaveformSignalRenderer(
                    type, options, ::WaveformRendererAbstract::Play);

    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    // if the signal renderer supports slip, we add it again, now for slip, together with the
    // other slip renderers
    if (waveformSignalRenderer && waveformSignalRenderer->supportsSlip()) {
        // The following renderer will add an overlay waveform if a slip is in progress
        addRenderer<WaveformRendererSlipMode>();
        addRenderer<WaveformRendererPreroll>(::WaveformRendererAbstract::Slip);
        addWaveformSignalRenderer(type, options, ::WaveformRendererAbstract::Slip);
        addRenderer<WaveformRenderBeat>(::WaveformRendererAbstract::Slip);
        addRenderer<WaveformRenderMark>(::WaveformRendererAbstract::Slip);
    }

    m_initSuccess = init();
}

WaveformWidget::~WaveformWidget() {
    makeCurrentIfNeeded();
    for (auto* pRenderer : std::as_const(m_rendererStack)) {
        delete pRenderer;
    }
    m_rendererStack.clear();
    doneCurrent();
}

allshader::WaveformRendererSignalBase*
WaveformWidget::addWaveformSignalRenderer(WaveformWidgetType::Type type,
        WaveformRendererSignalBase::Options options,
        ::WaveformRendererAbstract::PositionSource positionSource) {
    switch (type) {
    case ::WaveformWidgetType::Simple:
        return addRenderer<WaveformRendererSimple>();
    case ::WaveformWidgetType::RGB:
        if (options & allshader::WaveformRendererSignalBase::HighDetail) {
            return addRenderer<WaveformRendererTextured>(type, positionSource, options);
        }
        return addRenderer<WaveformRendererRGB>(positionSource, options);
    case ::WaveformWidgetType::HSV:
        return addRenderer<WaveformRendererHSV>();
    case ::WaveformWidgetType::Filtered:
        if (options & allshader::WaveformRendererSignalBase::HighDetail) {
            return addRenderer<WaveformRendererTextured>(type, positionSource, options);
        }
        return addRenderer<WaveformRendererFiltered>(false);
    case ::WaveformWidgetType::Stacked:
        if (options & allshader::WaveformRendererSignalBase::HighDetail) {
            return addRenderer<WaveformRendererTextured>(type, positionSource, options);
        } else {
            return addRenderer<WaveformRendererFiltered>(true); // true for RGB Stacked
        }
    default:
        break;
    }
    return nullptr;
}

mixxx::Duration WaveformWidget::render() {
    makeCurrentIfNeeded();
    paintGL();
    doneCurrent();
    // In the legacy widgets, this is used to "return timer for painter setup"
    // which is not relevant here. Also note that the return value is not used
    // at all, so it might be better to remove it everywhere. In the meantime.
    // we need to return something for API compatibility.
    return mixxx::Duration();
}

void WaveformWidget::paintGL() {
    if (shouldOnlyDrawBackground()) {
        if (!m_rendererStack.empty()) {
            m_rendererStack[0]->allshaderWaveformRenderer()->paintGL();
        }
    } else {
        for (auto* pRenderer : std::as_const(m_rendererStack)) {
            pRenderer->allshaderWaveformRenderer()->paintGL();
        }
    }
}

void WaveformWidget::castToQWidget() {
    m_widget = this;
}

void WaveformWidget::initializeGL() {
    for (auto* pRenderer : std::as_const(m_rendererStack)) {
        pRenderer->allshaderWaveformRenderer()->initializeGL();
    }
}

void WaveformWidget::resizeGL(int w, int h) {
    for (auto* pRenderer : std::as_const(m_rendererStack)) {
        pRenderer->allshaderWaveformRenderer()->resizeGL(w, h);
    }
}

void WaveformWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
}

void WaveformWidget::wheelEvent(QWheelEvent* pEvent) {
    QApplication::sendEvent(parentWidget(), pEvent);
    pEvent->accept();
}

void WaveformWidget::leaveEvent(QEvent* pEvent) {
    QApplication::sendEvent(parentWidget(), pEvent);
    pEvent->accept();
}

/* static */
int WaveformWidget::supportedOptions(WaveformWidgetType::Type type) {
    switch (type) {
    case WaveformWidgetType::Type::RGB:
        return WaveformRendererSignalBase::SplitStereoSignal |
                WaveformRendererSignalBase::HighDetail;
    case WaveformWidgetType::Type::Filtered:
        return WaveformRendererSignalBase::HighDetail;
    case WaveformWidgetType::Type::Stacked:
        return WaveformRendererSignalBase::HighDetail;
    default:
        break;
    }
    return 0;
}

/* static */
WaveformWidgetVars WaveformWidget::vars() {
    WaveformWidgetVars result;
    result.m_useGL = true;
    result.m_useGLES = true;
    result.m_useGLSL = true;
    result.m_category = WaveformWidgetCategory::AllShader;
    return result;
}

} // namespace allshader
