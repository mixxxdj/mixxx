#pragma once

#include <QList>
#include <QObject>

#include "effects/defs.h"

QT_FORWARD_DECLARE_CLASS(QDomDocument);

class VisibleEffectsList : public QObject {
    Q_OBJECT

  public:
    const QList<EffectManifestPointer>& getList() const {
        return m_list;
    }

    int indexOf(EffectManifestPointer pManifest) const;
    const EffectManifestPointer at(int index) const;
    const EffectManifestPointer next(const EffectManifestPointer pManifest) const;
    const EffectManifestPointer previous(const EffectManifestPointer pManifest) const;

    void setList(const QList<EffectManifestPointer>& newList);
    void readEffectsXml(const QDomDocument& doc,
            EffectsBackendManagerPointer pBackendManager);
    void saveEffectsXml(QDomDocument* pDoc,
            EffectsBackendManagerPointer pBackendManager);

  signals:
    void visibleEffectsListChanged();

  private:
    QList<EffectManifestPointer> readEffectsList(const QDomDocument& doc,
            EffectsBackendManagerPointer pBackendManager,
            const QString& xmlElementName);
    void saveEffectsListXml(QDomDocument* pDoc,
            const QList<EffectManifestPointer>& list,
            const QString& xmlElementName);

    QList<EffectManifestPointer> m_list;
};
