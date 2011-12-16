#include "waveformrendererabstract.h"

WaveformRendererAbstract::WaveformRendererAbstract( WaveformWidgetRenderer* waveformWidgetRenderer)
        : m_waveformRenderer(waveformWidgetRenderer),
          m_dirty(true) {
}

WaveformRendererAbstract::~WaveformRendererAbstract() {
}
