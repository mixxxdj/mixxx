#include "waveform/renderers/waveformrenderkeycurvebase.h"

#include "moc_waveformrenderkeycurvebase.cpp"
#include "waveform/renderers/waveformwidgetrenderer.h"

WaveformRenderKeyCurveBase::WaveformRenderKeyCurveBase(
        WaveformWidgetRenderer* pWaveformWidgetRenderer,
        bool updateImmediately)
        : WaveformRendererAbstract(pWaveformWidgetRenderer),
          m_updateImmediately(updateImmediately) {
}

void WaveformRenderKeyCurveBase::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
}

void WaveformRenderKeyCurveBase::onSetTrack() {
    if (m_updateImmediately) {
        updateKeyTextures();
    }
}

void WaveformRenderKeyCurveBase::onResize() {
    if (m_updateImmediately) {
        updateKeyTextures();
    }
}
