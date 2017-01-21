#include <set>
#include <QtDebug>

#include "waveformmarkset.h"
#include "engine/cuecontrol.h"
#include "control/controlobject.h"
#include "util/memory.h"

WaveformMarkSet::WaveformMarkSet()
    : m_iFirstHotCue(-1) {
}

WaveformMarkSet::~WaveformMarkSet() {
    clear();
}

void WaveformMarkSet::setup(const QString& group, const QDomNode& node,
                            const SkinContext& context,
                            const WaveformSignalColors& signalColors) {


#if QT_VERSION >= 0x040700
    m_marks.reserve(NUM_HOT_CUES);
#endif

    std::set<QString> controlItemSet;
    bool hasDefaultMark = false;

    QDomNode child = node.firstChild();
    QDomNode defaultChild;
    while (!child.isNull()) {
        if (child.nodeName() == "DefaultMark") {
            m_pDefaultMark = std::make_unique<WaveformMark>(group, child, context, signalColors);
            hasDefaultMark = true;
            defaultChild = child;
        } else if (child.nodeName() == "Mark") {
            WaveformMarkPointer pMark(new WaveformMark(group, child, context, signalColors));

            bool uniqueMark = true;
            if (pMark->isValid()) {
                // guarantee uniqueness even if there is a misdesigned skin
                QString item = context.selectString(child, "Control");
                if (!controlItemSet.insert(item).second) {
                    qWarning() << "WaveformRenderMark::setup - redefinition of" << item;
                    uniqueMark = false;
                }
            }
            if (uniqueMark) {
                m_marks.push_back(pMark);
            }
        }
        child = child.nextSibling();
    }

    if (NUM_HOT_CUES >= 1) {
        m_iFirstHotCue = m_marks.size();
    }

    // check if there is a default mark and compare declared
    // and to create all missing hot_cues
    if (hasDefaultMark) {
        for (int i = 1; i <= NUM_HOT_CUES; ++i) {
            QString hotCueControlItem = "hotcue_" + QString::number(i) + "_position";
            WaveformMarkPointer pMark(new WaveformMark(group, defaultChild, context, signalColors, i, hotCueControlItem));
            if (!pMark->isValid()) {
                continue;
            }

            if (controlItemSet.insert(hotCueControlItem).second) {
                //qDebug() << "WaveformRenderMark::setup - Automatic mark" << hotCueControlItem;
                pMark->setHotcueNumber(i);
                m_marks.push_back(pMark);
            }
        }
    }
}

WaveformMarkPointer WaveformMarkSet::getHotCueMark(int hotCue) const {
    DEBUG_ASSERT(hotCue >= 0);
    DEBUG_ASSERT(hotCue < NUM_HOT_CUES);
    return operator[](m_iFirstHotCue + hotCue);
}

void WaveformMarkSet::setHotCueMark(int hotCue, WaveformMarkPointer pMark) {
    m_marks[m_iFirstHotCue + hotCue] = pMark;
}
