#include "qml/qmlstemsmodel.h"

#include <QModelIndex>

#include "moc_qmlstemsmodel.cpp"

namespace mixxx {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {QmlStemsModel::LabelRole, "label"},
        {QmlStemsModel::ColorRole, "color"},
};
}

QmlStemsModel::QmlStemsModel(
        QObject* pParent)
        : QAbstractListModel(pParent) {
}

void QmlStemsModel::setStems(QList<StemInfo> stems) {
    beginResetModel();
    m_stems = std::move(stems);
    endResetModel();
}

QVariant QmlStemsModel::data(const QModelIndex& index, int role) const {
    if (index.row() < 0 || index.row() >= m_stems.size()) {
        return QVariant();
    }

    const StemInfo& stemInfo = m_stems.at(index.row());

    switch (role) {
    case QmlStemsModel::LabelRole: {
        return stemInfo.getLabel();
    }
    case QmlStemsModel::ColorRole:
        return stemInfo.getColor();
    default:
        return QVariant();
    }
}

int QmlStemsModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return m_stems.size();
}

QHash<int, QByteArray> QmlStemsModel::roleNames() const {
    return kRoleNames;
}

QVariant QmlStemsModel::get(int row) const {
    QModelIndex idx = index(row, 0);
    QVariantMap dataMap;
    for (auto it = kRoleNames.constBegin(); it != kRoleNames.constEnd(); it++) {
        dataMap.insert(it.value(), data(idx, it.key()));
    }
    return dataMap;
}

} // namespace qml
} // namespace mixxx
