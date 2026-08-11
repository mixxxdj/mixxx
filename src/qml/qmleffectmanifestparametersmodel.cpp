#include "qml/qmleffectmanifestparametersmodel.h"

#include <QModelIndex>

#include "effects/backends/effectmanifest.h"
#include "effects/effectparameter.h"
#include "effects/effectslot.h"
#include "moc_qmleffectmanifestparametersmodel.cpp"

namespace mixxx {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {QmlEffectManifestParametersModel::IdRole, "parameterId"},
        {QmlEffectManifestParametersModel::NameRole, "name"},
        {QmlEffectManifestParametersModel::ShortNameRole, "shortName"},
        {QmlEffectManifestParametersModel::DescriptionRole, "description"},
        {QmlEffectManifestParametersModel::TypeRole, "type"},
        {QmlEffectManifestParametersModel::ControlKeyRole, "controlKey"},
        {QmlEffectManifestParametersModel::LoadedRole, "loaded"},
};
}

QmlEffectManifestParametersModel::QmlEffectManifestParametersModel(
        EffectSlotPointer pEffectSlot,
        QObject* parent)
        : QAbstractListModel(parent),
          m_pEffectSlot(std::move(pEffectSlot)) {
}

EffectParameterPointer QmlEffectManifestParametersModel::loadedParameterForRow(
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

QVariant QmlEffectManifestParametersModel::data(const QModelIndex& index, int role) const {
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
    case QmlEffectManifestParametersModel::IdRole:
        return pParameter->id();
    case QmlEffectManifestParametersModel::NameRole:
        return pParameter->name();
    case QmlEffectManifestParametersModel::ShortNameRole:
        return pParameter->shortName();
    case QmlEffectManifestParametersModel::DescriptionRole:
        return pParameter->description();
    case QmlEffectManifestParametersModel::TypeRole:
        // TODO: Remove this cast, instead expose the enum directly using
        // Q_ENUM after #2618 has been merged.
        return static_cast<int>(pParameter->parameterType());
    case QmlEffectManifestParametersModel::ControlKeyRole: {
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
    case QmlEffectManifestParametersModel::LoadedRole:
        return static_cast<bool>(loadedParameterForRow(index.row()));
    default:
        return QVariant();
    }
}

int QmlEffectManifestParametersModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || !m_pEffectSlot->getManifest()) {
        return 0;
    }
    return m_pEffectSlot->getManifest()->parameters().size();
}

QHash<int, QByteArray> QmlEffectManifestParametersModel::roleNames() const {
    return kRoleNames;
}

QVariant QmlEffectManifestParametersModel::get(int row) const {
    QModelIndex idx = index(row, 0);
    QVariantMap dataMap;
    for (auto it = kRoleNames.constBegin(); it != kRoleNames.constEnd(); it++) {
        dataMap.insert(it.value(), data(idx, it.key()));
    }
    return dataMap;
}

} // namespace qml
} // namespace mixxx
