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
        {QmlEffectManifestParametersModel::ControlKeyRole, "controlKey"},
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
        // TODO: Remove this cast, instead expose the enum directly using
        // Q_ENUM after #2618 has been merged.
        return static_cast<int>(pParameter->controlHint());
    case QmlEffectManifestParametersModel::ControlKeyRole: {
        // FIXME: Unfortunately our effect parameter controls are messed up.
        // Even though we only have a single, ordered list of parameters, our
        // COs splits up this list into two distinct list (`parameter_N` and
        // `button_parameter_M`), and their indices don't match up with the
        // original list.
        //
        // For example, if you have 4 parameters (A: Knob, B: Button, C: Knob,
        // D: Knob), one would expect the following control keys:
        //    parameter1 -> A
        //    button_parameter2 -> B
        //    parameter3 -> C
        //    parameter4 -> D
        //
        // But in reality, this will lead to the following control keys:
        //    parameter1 -> A
        //    button_parameter1 -> B
        //    parameter2 -> C
        //    parameter3 -> D
        //
        // This  makes it extremely hard to show the parameters in the correct
        // order, because you also need to know how many parameters of the same
        // type are in that list.
        //
        // Due to backwards compatibility, we cannot fix this. This attempts to
        // solve this problem by letting the user fetch the appropriate key
        // from the model.
        if (pParameter->controlHint() == EffectManifestParameter::ControlHint::UNKNOWN) {
            return QString();
        }
        const bool isButton = pParameter->controlHint() ==
                EffectManifestParameter::ControlHint::TOGGLE_STEPPING;
        int keyNumber = 1;
        for (int i = 0; i < index.row(); i++) {
            const EffectManifestParameterPointer pPrevParameter = parameters.at(i);
            if (pPrevParameter->controlHint() == EffectManifestParameter::ControlHint::UNKNOWN) {
                continue;
            }
            if (isButton ==
                    (pPrevParameter->controlHint() ==
                            EffectManifestParameter::ControlHint::
                                    TOGGLE_STEPPING)) {
                keyNumber++;
            }
        }

        return (isButton ? QStringLiteral("button_parameter%1")
                         : QStringLiteral("parameter%1"))
                .arg(QString::number(keyNumber));
    }
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
