#include "effects/visibleeffectslist.h"

void VisibleEffectsList::setList(const QList<EffectManifestPointer>& newList) {
    m_list = newList;
    emit visibleEffectsListChanged();
}
