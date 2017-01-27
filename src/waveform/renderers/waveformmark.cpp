#include <QtDebug>

#include "skin/skincontext.h"
#include "waveform/renderers/waveformmarkproperties.h"

#include "waveformmark.h"

WaveformMark::WaveformMark(const QString& group,
                           const QDomNode& node,
                           const SkinContext& context,
                           const WaveformSignalColors& signalColors,
                           int hotCue,
                           QString item)
    : m_iHotCue(hotCue) {
    if(item.isEmpty())
        item = context.selectString(node, "Control");
    if (!item.isEmpty()) {
        m_pPointCos = std::make_unique<ControlProxy>(group, item);
    }
    m_properties = WaveformMarkProperties(node, context, signalColors);
}


void WaveformMark::connectSamplePositionChanged(const QObject *obj, const char *slt) const {
    m_pPointCos->connectValueChanged(obj, slt, Qt::AutoConnection);
}

