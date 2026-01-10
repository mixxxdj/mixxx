#include "waveformrendererabstract.h"

WaveformRendererAbstract::WaveformRendererAbstract(WaveformWidgetRenderer* waveformWidgetRenderer)
        : m_waveformRenderer(waveformWidgetRenderer),
          m_lastPlayMarkerPos(-1.0),
          m_dirty(true),
          m_scaleFactor(1.0) {
}

WaveformRendererAbstract::~WaveformRendererAbstract() {
}
