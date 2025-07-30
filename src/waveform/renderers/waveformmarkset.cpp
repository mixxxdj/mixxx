#include "waveformmarkset.h"

#include <QtDebug>
#include <set>

#include "util/defs.h"

WaveformMarkSet::WaveformMarkSet() {
}

WaveformMarkSet::~WaveformMarkSet() {
    clear();
}

void WaveformMarkSet::setup(const QString& group, const QDomNode& node,
                            const SkinContext& context,
                            const WaveformSignalColors& signalColors) {
    // + 3 for cue_point, loop_start_position and loop_end_position
    m_marks.reserve(kMaxNumberOfHotcues + 3);
    // Note: m_hotCueMarks does not support reserving space

    std::set<QString> controlItemSet;
    bool hasDefaultMark = false;

    QDomNode child = node.firstChild();
    QDomNode defaultChild;
    int priority = 0;
    while (!child.isNull()) {
        if (child.nodeName() == "DefaultMark") {
            m_pDefaultMark = WaveformMarkPointer::create(
                    group, child, context, --priority, signalColors);
            hasDefaultMark = true;
            defaultChild = child;
        } else if (child.nodeName() == "Mark") {
            auto pMark = WaveformMarkPointer::create(
                    group, child, context, --priority, signalColors);
            if (pMark->isValid()) {
                // guarantee uniqueness even if there is a misdesigned skin
                QString item = pMark->getItem();
                if (!controlItemSet.insert(item).second) {
                    qWarning() << "WaveformRenderMark::setup - redefinition of" << item;
                } else  {
                    addMark(pMark);
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
        for (int i = 0; i < kMaxNumberOfHotcues; ++i) {
            if (m_hotCueMarks.value(i).isNull()) {
                // qDebug() << "WaveformRenderMark::setup - Automatic mark" << hotCueControlItem;
                auto pMark = WaveformMarkPointer::create(
                        group, defaultChild, context, i, signalColors, i);
                m_marks.push_front(pMark);
                m_hotCueMarks.insert(pMark->getHotCue(), pMark);
            }
        }
    }
}

void WaveformMarkSet::setDefault(const QString& group,
        const DefaultMarkerStyle& model,
        const WaveformSignalColors& signalColors) {
    m_pDefaultMark = WaveformMarkPointer::create(

            group,
            model.positionControl,
            model.visibilityControl,
            model.textColor,
            model.markAlign,
            model.text,
            model.pixmapPath,
            model.iconPath,
            model.color,
            0,
            Cue::kNoHotCue,
            signalColors);
    for (int i = 0; i < kMaxNumberOfHotcues; ++i) {
        if (m_hotCueMarks.value(i).isNull()) {
            auto pMark = WaveformMarkPointer::create(

                    group,
                    model.positionControl,
                    model.visibilityControl,
                    model.textColor,
                    model.markAlign,
                    model.text,
                    model.pixmapPath,
                    model.iconPath,
                    model.color,
                    i,
                    i,
                    signalColors);
            m_marks.push_front(pMark);
            m_hotCueMarks.insert(pMark->getHotCue(), pMark);
        }
    }
}

WaveformMarkPointer WaveformMarkSet::getHotCueMark(int hotCue) const {
    return m_hotCueMarks.value(hotCue);
}

WaveformMarkPointer WaveformMarkSet::getDefaultMark() const {
    return m_pDefaultMark;
}

void WaveformMarkSet::setBreadth(float breadth) {
    for (auto& pMark : m_marks) {
        pMark->setBreadth(breadth);
    }
}

void WaveformMarkSet::update() {
    std::map<WaveformMarkSortKey, WaveformMarkPointer> map;
    for (const auto& pMark : std::as_const(m_marks)) {
        if (pMark->isValid() && pMark->isVisible()) {
            double samplePosition = pMark->getSamplePosition();
            if (samplePosition != Cue::kNoPosition) {
                // Create a stable key for sorting, because the WaveformMark's samplePosition is a
                // ControlObject which can change at any time by other threads. Such a change causes
                // another updateCues() call, rebuilding map.
                auto key = WaveformMarkSortKey(samplePosition, pMark->getPriority());
                map.emplace(key, pMark);
            }
        }
    }

    m_marksToRender.clear();
    m_marksToRender.reserve(static_cast<QList<WaveformMarkPointer>::size_type>(map.size()));
    std::transform(map.begin(),
            map.end(),
            std::back_inserter(m_marksToRender),
            [](auto const& pair) { return pair.second; });

    double prevSamplePosition = Cue::kNoPosition;

    // Avoid overlapping marks by increasing the level per alignment.
    // We take this into account when drawing the marks aligned at:
    // left top, right top, left bottom, right bottom.
    std::map<Qt::Alignment, int> levels;
    for (auto& pMark : m_marksToRender) {
        if (pMark->getSamplePosition() != prevSamplePosition) {
            prevSamplePosition = pMark->getSamplePosition();
            levels.clear();
        }
        pMark->setLevel(levels[pMark->m_align]++);
    }
}

WaveformMarkPointer WaveformMarkSet::findHoveredMark(
        QPoint pos, Qt::Orientation orientation) const {
    // Non-hotcue marks (intro/outro cues, main cue, loop in/out) are sorted
    // before hotcues in m_marksToRender so if there is a hotcue in the same
    // location, the hotcue gets rendered on top. When right clicking, the
    // the hotcue rendered on top must be assigned to m_pHoveredMark to show
    // the CueMenuPopup. To accomplish this, m_marksToRender is iterated in
    // reverse and the loop breaks as soon as m_pHoveredMark is set.
    for (auto it = m_marksToRender.crbegin(); it != m_marksToRender.crend(); ++it) {
        const WaveformMarkPointer& pMark = *it;
        if (pMark->contains(pos, orientation)) {
            return pMark;
        }
    }
    for (auto it = m_marksToRender.crbegin(); it != m_marksToRender.crend(); ++it) {
        const WaveformMarkPointer& pMark = *it;
        if (pMark->lineHovered(pos, orientation)) {
            return pMark;
        }
    }
    return nullptr;
}
