#include <set>
#include <QtDebug>

#include "waveformmarkset.h"
#include "engine/cuecontrol.h"
#include "control/controlobject.h"

WaveformMarkSet::WaveformMarkSet()
    : m_iFirstHotCue(-1) {
}

WaveformMarkSet::~WaveformMarkSet() {
    clear();
}

void WaveformMarkSet::setup(const QString& group, const QDomNode& node,
                            const SkinContext& context,
                            const WaveformSignalColors& signalColors) {

    clear();

#if QT_VERSION >= 0x040700
    m_marks.reserve(NUM_HOT_CUES);
#endif

    std::set<QString> controlItemSet;
    bool hasDefaultMark = false;

    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.nodeName() == "DefaultMark") {
            m_pDefaultMark->setup(group, child, context, signalColors);
            hasDefaultMark = true;
        } else if (child.nodeName() == "Mark") {
            m_marks.push_back(new WaveformMark());
            WaveformMark* mark = m_marks.back();
            mark->setup(group, child, context, signalColors);

            if (mark->m_pPointCos) {
                // guarantee uniqueness even if there is a misdesigned skin
                QString item = mark->m_pPointCos->getKey().item;
                if (!controlItemSet.insert(item).second) {
                    qWarning() << "WaveformRenderMark::setup - redefinition of" << item;
                    m_marks.removeAt(m_marks.size() - 1);
                }
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
            ControlObject* pHotcue = ControlObject::getControl(
                    ConfigKey(group, hotCueControlItem));
            if (pHotcue == NULL) {
                continue;
            }

            if (controlItemSet.insert(hotCueControlItem).second) {
                //qDebug() << "WaveformRenderMark::setup - Automatic mark" << hotCueControlItem;
                m_marks.push_back(new WaveformMark(m_pDefaultMark.get()));
                WaveformMark* mark = m_marks.back();
                mark->setKeyAndIndex(pHotcue->getKey(), i);
            }
        }
    }
}

void WaveformMarkSet::clear() {
    m_pDefaultMark = std::make_unique<WaveformMark>();
    qDeleteAll(m_marks);
    m_marks.clear();
}

const WaveformMark* WaveformMarkSet::getHotCueMark(int hotCue) {
    DEBUG_ASSERT(hotCue >= 0);
    DEBUG_ASSERT(hotCue < NUM_HOT_CUES);
    return m_marks[m_iFirstHotCue + hotCue];
}

void WaveformMarkSet::setHotCueMark(int hotCue, WaveformMark* mark) {
    delete m_marks[m_iFirstHotCue + hotCue];
    m_marks[m_iFirstHotCue + hotCue] = mark;
}
