#include "waveformmarkset.h"
#include "engine/cuecontrol.h"
#include "controlobject.h"

#include <set>
#include <QDebug>

WaveformMarkSet::WaveformMarkSet()
{
}

void WaveformMarkSet::setup( const QString& group, const QDomNode& node) {
    m_defaultMark = WaveformMark();
    m_marks.clear();

    m_defaultMark = WaveformMark();
    m_marks.clear();
#if QT_VERSION >= 0x040700
    m_marks.reserve(NUM_HOT_CUES);
#endif

    std::set<QString> hotCutSet;
    bool hasDefaultMark = false;

    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.nodeName() == "DefaultMark") {
            m_defaultMark.setup( group, child);
            hasDefaultMark = true;
        } else if (child.nodeName() == "Mark") {
            m_marks.push_back(WaveformMark());
            WaveformMark& mark = m_marks.back();
            mark.setup( group, child);
            //garante uniqueness even if there is misdesigned skin
            std::pair<std::set<QString>::iterator, bool> insertion;
            insertion = hotCutSet.insert(mark.m_pointControl->getKey().item);
            if( !insertion.second) {
                qWarning() << "WaveformRenderMark::setup - redefinition of "
                           << mark.m_pointControl->getKey().item;
                m_marks.removeAt( m_marks.size() - 1);
            }
        }
        child = child.nextSibling();
    }

    //check if there is a default mark and compare declared
    //and to create all missing hot_cues
    if(hasDefaultMark) {
        for( int i = 1; i < NUM_HOT_CUES; i++) {
            QString hotCueControlItem = "hotcue_" + QString::number(i) + "_position";
            std::pair<std::set<QString>::iterator, bool> insertion;
            insertion = hotCutSet.insert(hotCueControlItem);
            if( insertion.second) {
                //qDebug() << "WaveformRenderMark::setup - Automatic mark" << hotCueControlItem;
                m_marks.push_back(m_defaultMark);
                WaveformMark& mark = m_marks.back();
                mark.m_pointControl = ControlObject::getControl(ConfigKey(group, hotCueControlItem));
                mark.m_text = mark.m_text.arg(i);
            }
        }
    }
}
