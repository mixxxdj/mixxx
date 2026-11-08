#pragma once
#include <QAbstractListModel>
#include <QQmlEngine>
#include <memory>

#include "effects/defs.h"

class EffectsManager;

namespace mixxx {
namespace qml {

class QmlChainPresetModel : public QAbstractListModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(ChainPresetModel)
    QML_UNCREATABLE("Only accessible via Mixxx.EffectsManager preset model properties")
  public:
    enum class PresetType {
        Standard,
        Quick,
    };

    enum Roles {
        NameRole = Qt::UserRole + 1,
        ReadOnlyRole,
        PresetDisplayRole,
    };
    Q_ENUM(Roles)

    explicit QmlChainPresetModel(
            std::shared_ptr<EffectsManager> pEffectsManager,
            PresetType presetType,
            QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;

  private slots:
    void slotUpdated();

  private:
    const std::shared_ptr<EffectsManager> m_pEffectsManager;
    const EffectChainPresetManagerPointer m_pEffectChainPresetManager;
    const PresetType m_presetType;
    QList<EffectChainPresetPointer> m_effectChainPresets;
};

} // namespace qml
} // namespace mixxx
