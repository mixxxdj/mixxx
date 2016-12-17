#include <QtDebug>

#include "skin/skincontext.h"
#include "waveform/renderers/waveformmarkproperties.h"

#include "waveformmark.h"

WaveformMark::WaveformMark( const QString& group,
                            const QDomNode& node,
                            const SkinContext& context,
                            const WaveformSignalColors& signalColors,
                            int hotCue)
    : m_iHotCue(hotCue) {
    m_pPointCos = nullptr;
    QString item = context.selectString(node, "Control");
    if (!item.isEmpty()) {
        m_pPointCos = std::make_unique<ControlProxy>(group, item);
    }
    m_properties = WaveformMarkProperties(node, context, signalColors);
}

WaveformMark::WaveformMark(int hotCue)
    : m_iHotCue(hotCue){
}

void WaveformMark::connectPlayPosChanged(const QObject *obj, const char *slt) {
    m_pPointCos->connectValueChanged(obj, slt, Qt::AutoConnection);
}

