#pragma once

#include <QList>

#include "effects/effectmanifest.h"

class QDomDocument;

class VisibleEffectsList : public QObject {
    Q_OBJECT

  public:
    const QList<EffectManifestPointer>& getList() const {
        return m_list;
    }
    const EffectManifestPointer at(int index) const {
        return m_list.at(index);
    }

    void setList(const QList<EffectManifestPointer>& newList);
    void readEffectsXml(const QDomDocument& doc, EffectsBackendManagerPointer pBackendManager);
    void saveEffectsXml(QDomDocument* pDoc);

  signals:
    void visibleEffectsListChanged();

  private:
    QList<EffectManifestPointer> m_list;
};
