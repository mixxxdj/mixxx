#include "waveform/widgets/allshader/waveformwidget.h"

#include <QApplication>
#include <QWheelEvent>

#include "waveform/renderers/allshader/waveformrenderbackground.h"
#include "waveform/renderers/allshader/waveformrenderbeat.h"
#include "waveform/renderers/allshader/waveformrenderer.h"
#include "waveform/renderers/allshader/waveformrendererabstract.h"
#include "waveform/renderers/allshader/waveformrendererendoftrack.h"
#include "waveform/renderers/allshader/waveformrendererfiltered.h"
#include "waveform/renderers/allshader/waveformrendererhsv.h"
#include "waveform/renderers/allshader/waveformrendererlrrgb.h"
#include "waveform/renderers/allshader/waveformrendererpreroll.h"
#include "waveform/renderers/allshader/waveformrendererrgb.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"
#include "waveform/renderers/allshader/waveformrenderersimple.h"
#include "waveform/renderers/allshader/waveformrendererslipmode.h"
#include "waveform/renderers/allshader/waveformrenderertextured.h"
#include "waveform/renderers/allshader/waveformrendermark.h"
#include "waveform/renderers/allshader/waveformrendermarkrange.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "waveform/widgets/allshader/moc_waveformwidget.cpp"

namespace allshader {

void WaveformWidget::registerInfos() {
    static WaveformWidgetInfo<WaveformWidget> s_infoFiltered{
            WaveformWidgetType::AllShaderFilteredWaveform,
            tr("Filtered"),
            true,
            true,
            true,
            false,
            WaveformWidgetCategory::AllShader};
    static WaveformWidgetInfo<WaveformWidget> s_infoHSV{
            WaveformWidgetType::AllShaderHSVWaveform,
            tr("HSV"),
            true,
            true,
            true,
            false,
            WaveformWidgetCategory::AllShader};
    static WaveformWidgetInfo<WaveformWidget> s_infoLRRGB{
            WaveformWidgetType::AllShaderLRRGBWaveform,
            tr("RGB L/R"),
            true,
            true,
            true,
            false,
            WaveformWidgetCategory::AllShader};
    static WaveformWidgetInfo<WaveformWidget> s_infoRGBStacked{
            WaveformWidgetType::AllShaderRGBStackedWaveform,
            tr("RGB Stacked"),
            true,
            true,
            true,
            false,
            WaveformWidgetCategory::AllShader};
    static WaveformWidgetInfo<WaveformWidget> s_infoRGB{
            WaveformWidgetType::AllShaderRGBWaveform,
            tr("RGB"),
            true,
            true,
            true,
            false,
            WaveformWidgetCategory::AllShader};
    static WaveformWidgetInfo<WaveformWidget> s_infoSimple{
            WaveformWidgetType::AllShaderSimpleWaveform,
            tr("Simple"),
            true,
            true,
            true,
            false,
            WaveformWidgetCategory::AllShader};
    static WaveformWidgetInfo<WaveformWidget> s_infoTexturedFiltered{
            WaveformWidgetType::AllShaderTexturedFiltered,
            tr("Filtered"),
            true,
            true,
            true,
            true,
            WaveformWidgetCategory::AllShader};
    static WaveformWidgetInfo<WaveformWidget> s_infoTexturedRGB{
            WaveformWidgetType::AllShaderTexturedRGB,
            tr("RGB"),
            true,
            true,
            true,
            true,
            WaveformWidgetCategory::AllShader};
    static WaveformWidgetInfo<WaveformWidget> s_infoTexturedStacked{
            WaveformWidgetType::AllShaderTexturedStacked,
            tr("RGB Stacked"),
            true,
            true,
            true,
            true,
            WaveformWidgetCategory::AllShader};
}

WaveformWidget::WaveformWidget(const WaveformWidgetInfoBase& info,
        const QString& group,
        QWidget* parent)
        : WGLWidget(parent), WaveformWidgetAbstract(group), m_info(info) {
    addRenderer<WaveformRenderBackground>();
    addRenderer<WaveformRendererEndOfTrack>();
    addRenderer<WaveformRendererPreroll>();
    addRenderer<WaveformRenderMarkRange>();
    switch (m_info.m_type) {
    case WaveformWidgetType::AllShaderFilteredWaveform:
        addRenderer<WaveformRendererFiltered>(false);
        break;
    case WaveformWidgetType::AllShaderHSVWaveform:
        addRenderer<WaveformRendererHSV>();
        break;
    case WaveformWidgetType::AllShaderLRRGBWaveform:
        addRenderer<WaveformRendererLRRGB>();
        break;
    case WaveformWidgetType::AllShaderRGBStackedWaveform:
        addRenderer<WaveformRendererFiltered>(true); // true for RGB Stacked
        break;
    case WaveformWidgetType::AllShaderRGBWaveform:
        addRenderer<WaveformRendererRGB>();
        break;
    case WaveformWidgetType::AllShaderSimpleWaveform:
        addRenderer<WaveformRendererSimple>();
        break;
    case WaveformWidgetType::AllShaderTexturedFiltered:
        addRenderer<WaveformRendererTextured>(WaveformRendererTextured::Type::Filtered);
        break;
    case WaveformWidgetType::AllShaderTexturedRGB:
        addRenderer<WaveformRendererTextured>(WaveformRendererTextured::Type::RGB);
        break;
    case WaveformWidgetType::AllShaderTexturedStacked:
        addRenderer<WaveformRendererTextured>(WaveformRendererTextured::Type::Stacked);
        break;
    default:
        break;
    }
    addRenderer<WaveformRenderBeat>();
    addRenderer<WaveformRenderMark>();

    if (m_info.m_type == WaveformWidgetType::AllShaderRGBWaveform) {
        // The following renderer will add an overlay waveform if a slip is in progress
        addRenderer<WaveformRendererSlipMode>();
        addRenderer<WaveformRendererPreroll>(::WaveformRendererAbstract::Slip);
        addRenderer<WaveformRendererRGB>(::WaveformRendererAbstract::Slip);
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

void WaveformWidget::wheelEvent(QWheelEvent* pEvent) {
    QApplication::sendEvent(parentWidget(), pEvent);
    pEvent->accept();
}

void WaveformWidget::leaveEvent(QEvent* pEvent) {
    QApplication::sendEvent(parentWidget(), pEvent);
    pEvent->accept();
}

} // namespace allshader
