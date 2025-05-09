#include "qml/qmllibrarytreeviewmodel.h"

#include "library/sidebarmodel.h"
#include "moc_qmllibrarytreeviewmodel.cpp"
#include "util/assert.h"

namespace mixxx {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {Qt::DisplayRole, "label"},
        {QmlLibraryTreeviewModel::IconRole, "icon"},
};

} // namespace

QmlLibraryTreeviewModel::QmlLibraryTreeviewModel(SidebarModel* pModel, QObject* pParent)
        : QIdentityProxyModel(pParent) {
    // pModel->select();
    setSourceModel(pModel);
}

QVariant QmlLibraryTreeviewModel::data(const QModelIndex& proxyIndex, int role) const {
    if (!proxyIndex.isValid()) {
        return {};
    }

    VERIFY_OR_DEBUG_ASSERT(checkIndex(proxyIndex)) {
        return {};
    }

    if (proxyIndex.column() > 0) {
        return {};
    }

    int column = -1;
    switch (role) {
    case LabelRole:
        return QIdentityProxyModel::data(proxyIndex, Qt::DisplayRole);
        break;
    case IconRole:
        return QIdentityProxyModel::data(proxyIndex, SidebarModel::IconNameRole);
        break;
    default:
        return QIdentityProxyModel::data(proxyIndex, role);
        break;
    }

    return {};
}

int QmlLibraryTreeviewModel::columnCount(const QModelIndex& parent) const {
    return 1;
}
int QmlLibraryTreeviewModel::rowCount(const QModelIndex& parent) const {
    // VERIFY_OR_DEBUG_ASSERT(!parent.isValid()) {
    //     return 0;
    // }
    auto* const pSourceModel = static_cast<SidebarModel*>(sourceModel());
    VERIFY_OR_DEBUG_ASSERT(pSourceModel) {
        return 0;
    }
    return pSourceModel->rowCount(parent);
}

QModelIndex QmlLibraryTreeviewModel::index(int row, int column, const QModelIndex& parent) const {
    return QIdentityProxyModel::index(row, column, parent);
}

QModelIndex QmlLibraryTreeviewModel::parent(const QModelIndex& index) const {
    return QIdentityProxyModel::parent(index);
}

QHash<int, QByteArray> QmlLibraryTreeviewModel::roleNames() const {
    return kRoleNames;
}

QVariant QmlLibraryTreeviewModel::get(int row) const {
    QModelIndex idx = index(row, 0);
    QVariantMap dataMap;
    for (auto it = kRoleNames.constBegin(); it != kRoleNames.constEnd(); it++) {
        dataMap.insert(it.value(), data(idx, it.key()));
    }
    return dataMap;
}

} // namespace qml
} // namespace mixxx
