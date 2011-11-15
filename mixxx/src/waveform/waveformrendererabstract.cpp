#include "waveformrendererabstract.h"

WaveformRendererAbstract::WaveformRendererAbstract( WaveformWidgetRenderer* waveformWidgetRenderer)
        : m_waveformWidget(waveformWidgetRenderer),
          m_dirty(true) {
}

WaveformRendererAbstract::~WaveformRendererAbstract() {
}
