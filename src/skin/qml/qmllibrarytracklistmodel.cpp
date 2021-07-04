#include "skin/qml/qmllibrarytracklistmodel.h"

namespace mixxx {
namespace skin {
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

QVariant QmlLibraryTrackListModel::data(const QModelIndex& proxyIndex, int role) const {
    if (!proxyIndex.isValid()) {
        return QVariant();
    }

    VERIFY_OR_DEBUG_ASSERT(checkIndex(proxyIndex)) {
        return QVariant();
    }

    const auto pSourceModel = static_cast<LibraryTableModel*>(sourceModel());
    VERIFY_OR_DEBUG_ASSERT(pSourceModel) {
        return QVariant();
    }

    if (proxyIndex.column() > 0) {
        return QVariant();
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
            return QVariant();
        }
        return QUrl::fromLocalFile(location);
    }
    default:
        break;
    }

    if (column < 0) {
        return QVariant();
    }

    return QIdentityProxyModel::data(proxyIndex.siblingAtColumn(column), Qt::DisplayRole);
}

int QmlLibraryTrackListModel::columnCount(const QModelIndex& parent) const {
    // This is a list model, i.e. entries without a parent have exactly one column.
    if (!parent.isValid()) {
        return 1;
    }

    // This is not a tree, so all indices with a parent don't have any columns.
    return 0;
}

QHash<int, QByteArray> QmlLibraryTrackListModel::roleNames() const {
    return kRoleNames;
}

} // namespace qml
} // namespace skin
} // namespace mixxx
