#pragma once
#include <QAbstractListModel>
#include <QtQml>
#include <memory>

#include "effects/effectsmanager.h"

namespace mixxx {
namespace qml {

class QmlVisibleEffectsModel : public QAbstractListModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(VisibleEffectsModel)
    QML_UNCREATABLE("Only accessible via Mixxx.EffectsManager.visibleEffectsModel")
  public:
    enum Roles {
        EffectIdRole = Qt::UserRole + 1,
    };
    Q_ENUM(Roles)

    explicit QmlVisibleEffectsModel(
            std::shared_ptr<EffectsManager> pEffectsManager,
            QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;

  private slots:
    void slotVisibleEffectsUpdated();

  private:
    const std::shared_ptr<EffectsManager> m_pEffectsManager;
    QList<EffectManifestPointer> m_visibleEffectManifests;
};

} // namespace qml
} // namespace mixxx
