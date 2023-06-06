#include "qml/qmllibrarytracklistmodel.h"

#include "library/librarytablemodel.h"
#include "qml/asyncimageprovider.h"

namespace mixxx {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {QmlLibraryTrackListModel::AlbumArtistRole, "albumArtist"},
        {QmlLibraryTrackListModel::AlbumRole, "album"},
        {QmlLibraryTrackListModel::ArtistRole, "artist"},
        {QmlLibraryTrackListModel::BitrateRole, "bitrate"},
        {QmlLibraryTrackListModel::BpmLockRole, "bpmLock"},
        {QmlLibraryTrackListModel::BpmRole, "bpm"},
        {QmlLibraryTrackListModel::ColorRole, "color"},
        {QmlLibraryTrackListModel::CommentRole, "comment"},
        {QmlLibraryTrackListModel::ComposerRole, "composer"},
        {QmlLibraryTrackListModel::CoverArtColorRole, "coverArtColor"},
        {QmlLibraryTrackListModel::CoverArtUrlRole, "coverArtUrl"},
        {QmlLibraryTrackListModel::DatetimeAddedRole, "datetimeAdded"},
        {QmlLibraryTrackListModel::DeletedRole, "deleted"},
        {QmlLibraryTrackListModel::DurationSecondsRole, "durationSeconds"},
        {QmlLibraryTrackListModel::FileTypeRole, "fileType"},
        {QmlLibraryTrackListModel::FileUrlRole, "fileUrl"},
        {QmlLibraryTrackListModel::GenreRole, "genre"},
        {QmlLibraryTrackListModel::GroupingRole, "grouping"},
        {QmlLibraryTrackListModel::KeyIdRole, "keyIdRole"},
        {QmlLibraryTrackListModel::KeyRole, "key"},
        {QmlLibraryTrackListModel::LastPlayedAtRole, "lastPlayedAt"},
        {QmlLibraryTrackListModel::PlayedRole, "played"},
        {QmlLibraryTrackListModel::RatingRole, "rating"},
        {QmlLibraryTrackListModel::ReplayGainRole, "ReplayGain"},
        {QmlLibraryTrackListModel::TimesPlayedRole, "timesPlayed"},
        {QmlLibraryTrackListModel::TitleRole, "title"},
        {QmlLibraryTrackListModel::TrackNumberRole, "trackNumber"},
        {QmlLibraryTrackListModel::YearRole, "year"},
};

QColor colorFromRgbCode(double colorValue) {
    if (colorValue < 0 || colorValue > 0xFFFFFF) {
        return {};
    }

    QRgb rgbValue = static_cast<QRgb>(colorValue) | 0xFF000000;
    return QColor(rgbValue);
}
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
    case AlbumArtistRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST);
        break;
    case AlbumRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM);
        break;
    case ArtistRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST);
        break;
    case BitrateRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE);
        break;
    case BpmLockRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK);
        break;
    case BpmRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM);
        break;
    case ColorRole: {
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COLOR);
        const double colorValue = QIdentityProxyModel::data(
                proxyIndex.siblingAtColumn(column), Qt::DisplayRole)
                                          .toDouble();
        return colorFromRgbCode(colorValue);
        break;
    }
    case CommentRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT);
        break;
    case ComposerRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER);
        break;
    case CoverArtUrlRole: {
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION);
        const QString location = QIdentityProxyModel::data(
                proxyIndex.siblingAtColumn(column), Qt::DisplayRole)
                                         .toString();
        if (location.isEmpty()) {
            return {};
        }

        return AsyncImageProvider::trackLocationToCoverArtUrl(location);
    }
    case CoverArtColorRole: {
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART_COLOR);
        const double colorValue = QIdentityProxyModel::data(
                proxyIndex.siblingAtColumn(column), Qt::DisplayRole)
                                          .toDouble();
        return colorFromRgbCode(colorValue);
        break;
    }
    case DatetimeAddedRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED);
        break;
    case DeletedRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_MIXXXDELETED);
        break;
    case DurationSecondsRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION);
        break;
    case FileTypeRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE);
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
    case GenreRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GENRE);
        break;
    case GroupingRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING);
        break;
    case KeyIdRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY_ID);
        break;
    case KeyRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY);
        break;
    case LastPlayedAtRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LAST_PLAYED_AT);
        break;
    case PlayedRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PLAYED);
        break;
    case RatingRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING);
        break;
    case ReplayGainRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN);
        break;
    case TimesPlayedRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED);
        break;
    case TitleRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
        break;
    case TrackNumberRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER);
        break;
    case YearRole:
        column = pSourceModel->fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR);
        break;
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
