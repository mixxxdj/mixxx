#pragma once

#include <QList>

#include "effects/effectmanifest.h"

class VisibleEffectsList : public QObject {
    Q_OBJECT

  public:
    const QList<EffectManifestPointer>& getList() const {
        return m_list;
    }
    void setList(const QList<EffectManifestPointer>& newList);

  signals:
    void visibleEffectsListChanged();

  private:
    QList<EffectManifestPointer> m_list;
};
