#include "skin/qml/qmleffectmanifestparametersmodel.h"

#include <QModelIndex>

#include "effects/effectmanifest.h"

namespace mixxx {
namespace skin {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {QmlEffectManifestParametersModel::IdRole, "id"},
        {QmlEffectManifestParametersModel::NameRole, "name"},
        {QmlEffectManifestParametersModel::ShortNameRole, "shortName"},
        {QmlEffectManifestParametersModel::DescriptionRole, "description"},
        {QmlEffectManifestParametersModel::ControlHintRole, "controlHint"},
};
}

QmlEffectManifestParametersModel::QmlEffectManifestParametersModel(
        EffectManifestPointer pEffectManifest,
        QObject* parent)
        : QAbstractListModel(parent), m_pEffectManifest(pEffectManifest) {
}

QVariant QmlEffectManifestParametersModel::data(const QModelIndex& index, int role) const {
    const QList<EffectManifestParameterPointer>& parameters = m_pEffectManifest->parameters();
    if (index.row() >= parameters.size()) {
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
    case QmlEffectManifestParametersModel::ControlHintRole:
        return static_cast<int>(pParameter->controlHint());
    default:
        return QVariant();
    }
}

int QmlEffectManifestParametersModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    // Add +1 because we also include "no effect" in the model
    return m_pEffectManifest->parameters().size();
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
} // namespace skin
} // namespace mixxx
