#include "waveform/renderers/waveformrenderbpmcurvebase.h"

#include "moc_waveformrenderbpmcurvebase.cpp"
#include "waveform/renderers/waveformwidgetrenderer.h"

WaveformRenderBpmCurveBase::WaveformRenderBpmCurveBase(
        WaveformWidgetRenderer* pWaveformWidgetRenderer,
        bool updateImmediately)
        : WaveformRendererAbstract(pWaveformWidgetRenderer),
          m_updateImmediately(updateImmediately) {
}

void WaveformRenderBpmCurveBase::setup(const QDomNode& node, const SkinContext& context) {
    Q_UNUSED(node);
    Q_UNUSED(context);
}

void WaveformRenderBpmCurveBase::onSetTrack() {
    if (m_updateImmediately) {
        updateBpmTextures();
    }
}

void WaveformRenderBpmCurveBase::onResize() {
    if (m_updateImmediately) {
        updateBpmTextures();
    }
}
