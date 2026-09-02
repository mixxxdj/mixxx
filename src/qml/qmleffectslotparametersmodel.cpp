#include "qml/qmleffectslotparametersmodel.h"

#include <QModelIndex>

#include "effects/backends/effectmanifest.h"
#include "effects/effectparameter.h"
#include "effects/effectslot.h"
#include "moc_qmleffectslotparametersmodel.cpp"

namespace mixxx {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {QmlEffectSlotParametersModel::IdRole, "parameterId"},
        {QmlEffectSlotParametersModel::NameRole, "name"},
        {QmlEffectSlotParametersModel::ShortNameRole, "shortName"},
        {QmlEffectSlotParametersModel::DescriptionRole, "description"},
        {QmlEffectSlotParametersModel::TypeRole, "type"},
        {QmlEffectSlotParametersModel::ControlKeyRole, "controlKey"},
        {QmlEffectSlotParametersModel::LoadedRole, "loaded"},
        {QmlEffectSlotParametersModel::UnitStringRole, "unitString"},
};
}

QmlEffectSlotParametersModel::QmlEffectSlotParametersModel(
        EffectSlotPointer pEffectSlot,
        QObject* parent)
        : QAbstractListModel(parent),
          m_pEffectSlot(std::move(pEffectSlot)) {
    connect(m_pEffectSlot.get(),
            &EffectSlot::effectChanged,
            this,
            &QmlEffectSlotParametersModel::resetModel);
    connect(m_pEffectSlot.get(),
            &EffectSlot::parametersChanged,
            this,
            &QmlEffectSlotParametersModel::updateParameterState);
}

void QmlEffectSlotParametersModel::resetModel() {
    beginResetModel();
    endResetModel();
}

void QmlEffectSlotParametersModel::updateParameterState() {
    const int parameterCount = rowCount({});
    if (parameterCount == 0) {
        return;
    }
    emit dataChanged(index(0, 0),
            index(parameterCount - 1, 0),
            {ControlKeyRole, LoadedRole});
}

EffectParameterPointer QmlEffectSlotParametersModel::loadedParameterForRow(
        int row, int* pSlotNumber) const {
    const auto pManifest = m_pEffectSlot->getManifest();
    if (!pManifest || row < 0 || row >= pManifest->parameters().size()) {
        return nullptr;
    }
    const auto pManifestParameter = pManifest->parameters().at(row);
    const auto loadedParameters =
            m_pEffectSlot->getLoadedParameters().value(pManifestParameter->parameterType());
    for (int index = 0; index < loadedParameters.size(); ++index) {
        const auto& pParameter = loadedParameters.at(index);
        if (pParameter->manifest()->id() == pManifestParameter->id()) {
            if (pSlotNumber) {
                *pSlotNumber = index + 1;
            }
            return pParameter;
        }
    }
    return nullptr;
}

QVariant QmlEffectSlotParametersModel::data(const QModelIndex& index, int role) const {
    const auto pManifest = m_pEffectSlot->getManifest();
    if (!pManifest) {
        return QVariant();
    }
    const QList<EffectManifestParameterPointer>& parameters = pManifest->parameters();
    if (!index.isValid() || index.row() < 0 || index.row() >= parameters.size()) {
        return QVariant();
    }

    const EffectManifestParameterPointer pParameter = parameters.at(index.row());
    switch (role) {
    case QmlEffectSlotParametersModel::IdRole:
        return pParameter->id();
    case QmlEffectSlotParametersModel::NameRole:
        return pParameter->name();
    case QmlEffectSlotParametersModel::ShortNameRole:
        return pParameter->shortName();
    case QmlEffectSlotParametersModel::DescriptionRole:
        return pParameter->description();
    case QmlEffectSlotParametersModel::TypeRole:
        return QVariant::fromValue(
                static_cast<ParameterType>(pParameter->parameterType()));
    case QmlEffectSlotParametersModel::ControlKeyRole: {
        int keyNumber = 0;
        if (!loadedParameterForRow(index.row(), &keyNumber)) {
            return QString();
        }
        const bool isButton = pParameter->parameterType() ==
                EffectManifestParameter::ParameterType::Button;
        return (isButton ? QStringLiteral("button_parameter%1")
                         : QStringLiteral("parameter%1"))
                .arg(QString::number(keyNumber));
    }
    case QmlEffectSlotParametersModel::LoadedRole:
        return static_cast<bool>(loadedParameterForRow(index.row()));
    case QmlEffectSlotParametersModel::UnitStringRole:
        return pParameter->unitString();
    default:
        return QVariant();
    }
}

int QmlEffectSlotParametersModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || !m_pEffectSlot->getManifest()) {
        return 0;
    }
    return m_pEffectSlot->getManifest()->parameters().size();
}

QHash<int, QByteArray> QmlEffectSlotParametersModel::roleNames() const {
    return kRoleNames;
}

QVariant QmlEffectSlotParametersModel::get(int row) const {
    QModelIndex idx = index(row, 0);
    QVariantMap dataMap;
    for (auto it = kRoleNames.constBegin(); it != kRoleNames.constEnd(); it++) {
        dataMap.insert(it.value(), data(idx, it.key()));
    }
    return dataMap;
}

} // namespace qml
} // namespace mixxx
