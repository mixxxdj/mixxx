#include "qml/qmllibrarytracklistmodel.h"

#include "library/librarytablemodel.h"
#include "moc_qmllibrarytracklistmodel.cpp"

namespace mixxx {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {QmlLibraryTrackListModel::TitleRole, "title"},
        {QmlLibraryTrackListModel::ArtistRole, "artist"},
        {QmlLibraryTrackListModel::AlbumRole, "album"},
        {QmlLibraryTrackListModel::AlbumArtistRole, "albumArtist"},
        {QmlLibraryTrackListModel::FileUrlRole, "fileUrl"},
};
}

QmlLibraryTrackListModel::QmlLibraryTrackListModel(LibraryTableModel* pModel, QObject* pParent)
        : QIdentityProxyModel(pParent) {
    pModel->select();
    setSourceModel(pModel);
}

QVariant QmlLibraryTrackListModel::data(const QModelIndex& proxyIndex, int role) const {
    if (!proxyIndex.isValid()) {
        return {};
    }

    VERIFY_OR_DEBUG_ASSERT(checkIndex(proxyIndex)) {
        return {};
    }

    const auto pSourceModel = static_cast<LibraryTableModel*>(sourceModel());
    VERIFY_OR_DEBUG_ASSERT(pSourceModel) {
        return {};
    }

    if (proxyIndex.column() > 0) {
        return {};
    }

    int column = -1;
    switch (role) {
    case TitleRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
        break;
    case ArtistRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST);
        break;
    case AlbumRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM);
        break;
    case AlbumArtistRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST);
        break;
    case FileUrlRole: {
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION);
        const QString location = QIdentityProxyModel::data(
                proxyIndex.siblingAtColumn(column), Qt::DisplayRole)
                                         .toString();
        if (location.isEmpty()) {
            return {};
        }
        return QUrl::fromLocalFile(location);
    }
    default:
        break;
    }

    if (column < 0) {
        return {};
    }

    return QIdentityProxyModel::data(proxyIndex.siblingAtColumn(column), Qt::DisplayRole);
}

int QmlLibraryTrackListModel::columnCount(const QModelIndex& parent) const {
    // This is a list model, i.e. no entries have a parent.
    VERIFY_OR_DEBUG_ASSERT(!parent.isValid()) {
        return 0;
    }

    // There is exactly one column. All data is exposed as roles.
    return 1;
}

QHash<int, QByteArray> QmlLibraryTrackListModel::roleNames() const {
    return kRoleNames;
}

QVariant QmlLibraryTrackListModel::get(int row) const {
    QModelIndex idx = index(row, 0);
    QVariantMap dataMap;
    for (auto it = kRoleNames.constBegin(); it != kRoleNames.constEnd(); it++) {
        dataMap.insert(it.value(), data(idx, it.key()));
    }
    return dataMap;
}

} // namespace qml
} // namespace mixxx
