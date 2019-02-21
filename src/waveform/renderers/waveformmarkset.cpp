#include <set>
#include <QtDebug>

#include "waveformmarkset.h"
#include "engine/controls/cuecontrol.h"
#include "control/controlobject.h"
#include "util/memory.h"

WaveformMarkSet::WaveformMarkSet() {
}

WaveformMarkSet::~WaveformMarkSet() {
    clear();
}

void WaveformMarkSet::setup(const QString& group, const QDomNode& node,
                            const SkinContext& context,
                            const WaveformSignalColors& signalColors) {

    m_marks.reserve(NUM_HOT_CUES + 3); // + 3 for cue_point, loop_start_position and loop_end_position
    // Note: m_hotCueMarks does not support reserving space

    std::set<QString> controlItemSet;
    bool hasDefaultMark = false;

    QDomNode child = node.firstChild();
    QDomNode defaultChild;
    while (!child.isNull()) {
        if (child.nodeName() == "DefaultMark") {
            m_pDefaultMark = WaveformMarkPointer(new WaveformMark(group, child, context, signalColors));
            hasDefaultMark = true;
            defaultChild = child;
        } else if (child.nodeName() == "Mark") {
            WaveformMarkPointer pMark(new WaveformMark(group, child, context, signalColors));
            if (pMark->isValid()) {
                // guarantee uniqueness even if there is a misdesigned skin
                QString item = pMark->getItem();
                if (!controlItemSet.insert(item).second) {
                    qWarning() << "WaveformRenderMark::setup - redefinition of" << item;
                } else  {
                    m_marks.push_back(pMark);
                    if (pMark->getHotCue() >= 0) {
                        m_hotCueMarks.insert(pMark->getHotCue(), pMark);
                    }
                }
            }
        }
        child = child.nextSibling();
    }

    // check if there is a default mark and compare declared
    // and to create all missing hot_cues
    if (hasDefaultMark) {
        for (int i = 0; i < NUM_HOT_CUES; ++i) {
            if (m_hotCueMarks.value(i).isNull()) {
                //qDebug() << "WaveformRenderMark::setup - Automatic mark" << hotCueControlItem;
                WaveformMarkPointer pMark(new WaveformMark(group, defaultChild, context, signalColors, i));
                m_marks.push_back(pMark);
                m_hotCueMarks.insert(pMark->getHotCue(), pMark);
            }
        }
    }
}

WaveformMarkPointer WaveformMarkSet::getHotCueMark(int hotCue) const {
    return m_hotCueMarks.value(hotCue);
}

WaveformMarkPointer WaveformMarkSet::getDefaultMark() const {
    return m_pDefaultMark;
}
