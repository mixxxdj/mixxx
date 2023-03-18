#include "waveform/renderers/qopengl/waveformrenderbackground.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

using namespace qopengl;

WaveformRenderBackground::WaveformRenderBackground(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRenderer(waveformWidgetRenderer),
          m_backgroundColor(0, 0, 0) {
}

WaveformRenderBackground::~WaveformRenderBackground() {
}

void WaveformRenderBackground::setup(const QDomNode& node,
        const SkinContext& context) {
    m_backgroundColor = m_waveformRenderer->getWaveformSignalColors()->getBgColor();

    QString backgroundPixmapPath = context.selectString(node, "BgPixmap");
    if (!backgroundPixmapPath.isEmpty()) {
        qWarning() << "WaveformView BgPixmap is not supported by qopengl::WaveformRenderBackground";
    }
}

void WaveformRenderBackground::renderGL() {
    glClearColor(static_cast<float>(m_backgroundColor.redF()),
            static_cast<float>(m_backgroundColor.greenF()),
            static_cast<float>(m_backgroundColor.blueF()),
            1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}
