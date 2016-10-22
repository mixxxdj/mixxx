#include <QtDebug>

#include "skin/skincontext.h"
#include "waveform/renderers/waveformmarkproperties.h"

#include "waveformmark.h"

WaveformMark::WaveformMark(int hotCue)
    : m_iHotCue(hotCue) {
}

void WaveformMark::reset(int hotCue) {
    WaveformMark(hotCue).swap(*this);
}

void WaveformMark::setup(const QString& group, const QDomNode& node,
                         const SkinContext& context,
                         const WaveformSignalColors& signalColors) {
    QString item = context.selectString(node, "Control");
    if (!item.isEmpty()) {
        m_pPointCos = std::make_unique<ControlProxy>(group, item);
    }

    m_properties = WaveformMarkProperties(node, context, signalColors);
}
