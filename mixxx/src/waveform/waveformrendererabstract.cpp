#include "waveformrendererabstract.h"

WaveformRendererAbstract::WaveformRendererAbstract( WaveformWidgetRenderer* waveformWidget) :
    m_waveformWidget( waveformWidget),
    m_dirty(true)
{
}
