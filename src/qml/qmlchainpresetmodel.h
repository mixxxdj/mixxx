#pragma once
#include <QAbstractListModel>
#include <QQmlEngine>

#include "effects/defs.h"

namespace mixxx {
namespace qml {

class QmlChainPresetModel : public QAbstractListModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(ChainPresetModel)
    QML_UNCREATABLE("Only accessible via Mixxx.EffectsManager.quickEffectPresetsModel")
  public:
    enum Roles {
        EffectIdRole = Qt::UserRole + 1,
    };
    Q_ENUM(Roles)

    explicit QmlChainPresetModel(
            EffectChainPresetManagerPointer effectChainPresetManager,
            QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;

  private slots:
    void slotUpdated();

  private:
    const EffectChainPresetManagerPointer m_pEffectChainPresetManager;
    QList<EffectChainPresetPointer> m_effectChainPresets;
};

} // namespace qml
} // namespace mixxx
