#include "waveformrendererabstract.h"

WaveformRendererAbstract::WaveformRendererAbstract(WaveformWidgetRenderer* pWaveformWidgetRenderer)
        : m_pWaveformRenderer(pWaveformWidgetRenderer),
          m_lastPlayMarkerPos(-1.0),
          m_dirty(true),
          m_scaleFactor(1.0) {
}

WaveformRendererAbstract::~WaveformRendererAbstract() {
}
