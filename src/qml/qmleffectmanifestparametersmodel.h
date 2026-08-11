#pragma once
#include <QAbstractListModel>
#include <QQmlEngine>
#include <memory>

#include "effects/effectsmanager.h"

namespace mixxx {
namespace qml {

class QmlEffectManifestParametersModel : public QAbstractListModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(EffectManifestParametersModel)
    QML_UNCREATABLE("Only accessible via Mixxx.EffectSlot.parametersModel")
  public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ShortNameRole,
        DescriptionRole,
        TypeRole,
        ControlKeyRole,
        LoadedRole,
    };
    Q_ENUM(Roles)

    explicit QmlEffectManifestParametersModel(
            EffectSlotPointer pEffectSlot,
            QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;

  private:
    EffectParameterPointer loadedParameterForRow(int row, int* pSlotNumber = nullptr) const;

    const EffectSlotPointer m_pEffectSlot;
};

} // namespace qml
} // namespace mixxx
