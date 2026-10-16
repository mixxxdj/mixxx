#pragma once
#include <QAbstractListModel>
#include <QFont>
#include <QQmlEngine>
#include <QSortFilterProxyModel>
#include <memory>

#include "effects/backends/effectmanifestparameter.h"
#include "effects/effectsmanager.h"

namespace mixxx {
namespace qml {

class QmlEffectSlotParametersModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* orderedModel READ orderedModel CONSTANT)
    Q_DISABLE_COPY_MOVE(QmlEffectSlotParametersModel)
    QML_NAMED_ELEMENT(EffectSlotParametersModel)
    QML_UNCREATABLE("Only accessible via Mixxx.EffectSlot.parametersModel")
  public:
    enum ParameterType {
        Knob = static_cast<int>(EffectManifestParameter::ParameterType::Knob),
        Button = static_cast<int>(EffectManifestParameter::ParameterType::Button),
    };
    Q_ENUM(ParameterType)

    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        ShortNameRole,
        DescriptionRole,
        TypeRole,
        ControlKeyRole,
        LoadedRole,
        UnitStringRole,
        MaximumRole,
        NeutralPointRole,
        DisplayOrderRole,
    };
    Q_ENUM(Roles)

    explicit QmlEffectSlotParametersModel(
            EffectSlotPointer pEffectSlot,
            QObject* parent = nullptr);

    QAbstractItemModel* orderedModel() {
        return &m_orderedModel;
    }

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant get(int row) const;
    Q_INVOKABLE QString formatNumber(double value) const;
    Q_INVOKABLE int labelWidth(const QString& label,
            double maximum,
            const QString& unitString,
            const QFont& font,
            bool useApplicationFont) const;

  private:
    void resetModel();
    void updateParameterState();
    EffectParameterPointer loadedParameterForRow(int row, int* pSlotNumber = nullptr) const;

    const EffectSlotPointer m_pEffectSlot;
    QSortFilterProxyModel m_orderedModel;
};

} // namespace qml
} // namespace mixxx
