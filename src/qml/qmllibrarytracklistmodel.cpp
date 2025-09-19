#include "qml/qmllibrarytracklistmodel.h"

#include <qnamespace.h>

#include <QObject>
#include <QQmlEngine>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVariant>

#include "library/basetracktablemodel.h"
#include "library/columncache.h"
#include "moc_qmllibrarytracklistmodel.cpp"
#include "qml/asyncimageprovider.h"
#include "qml/qmllibrarytracklistcolumn.h"
#include "qml_owned_ptr.h"
#include "qmltrackproxy.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/parented_ptr.h"

namespace mixxx {
namespace qml {
namespace {
const QHash<int, QByteArray> kRoleNames = {
        {Qt::DisplayRole, "display"},
        {Qt::DecorationRole, "decoration"},
        {QmlLibraryTrackListModel::Delegate, "delegate"},
        {QmlLibraryTrackListModel::Track, "track"},
        {QmlLibraryTrackListModel::FileURL, "file_url"},
        {QmlLibraryTrackListModel::CoverArt, "cover_art"},
};

QColor colorFromRgbCode(double colorValue) {
    if (colorValue < 0 || colorValue > 0xFFFFFF) {
        return {};
    }

    QRgb rgbValue = static_cast<QRgb>(colorValue) | 0xFF000000;
    return QColor(rgbValue);
}
} // namespace

QmlLibraryTrackListModel::QmlLibraryTrackListModel(
        const QList<QmlLibraryTrackListColumn*>& librarySource,
        QAbstractItemModel* pModel,
        QObject* pParent)
        : QIdentityProxyModel(pParent),
          m_columns() {
    m_columns.reserve(librarySource.size());
    for (const auto* pColumn : std::as_const(librarySource)) {
        m_columns.emplace_back(make_parented<QmlLibraryTrackListColumn>(this,
                pColumn->label(),
                pColumn->fillSpan(),
                pColumn->columnIdx(),
                pColumn->preferredWidth(),
                pColumn->delegate(),
                pColumn->role()));
    }

    auto* pTrackModel = dynamic_cast<TrackModel*>(pModel);
    VERIFY_OR_DEBUG_ASSERT(pTrackModel) {
        return;
    }
    pTrackModel->select();
    setSourceModel(pModel);
}

QVariant QmlLibraryTrackListModel::data(const QModelIndex& proxyIndex, int role) const {
    if (!proxyIndex.isValid()) {
        return {};
    }

    VERIFY_OR_DEBUG_ASSERT(checkIndex(proxyIndex)) {
        return {};
    }
    auto columnIdx = proxyIndex.column();
    VERIFY_OR_DEBUG_ASSERT(columnIdx >= 0 || columnIdx < m_columns.size()) {
        return {};
    }

    auto* const pTrackTableModel = qobject_cast<BaseTrackTableModel*>(sourceModel());
    auto* const pTrackModel = dynamic_cast<TrackModel*>(sourceModel());

    const auto& pColumn = m_columns[columnIdx];

    switch (role) {
    case Track: {
        if (pTrackModel == nullptr) {
            return {};
        }
        auto pTrack = make_qml_owned<QmlTrackProxy>(pTrackModel->getTrack(
                QIdentityProxyModel::mapToSource(proxyIndex)));
        return QVariant::fromValue(pTrack.get());
    }
    case Qt::DecorationRole: {
        if (pTrackTableModel == nullptr) {
            return {};
        };
        return colorFromRgbCode(QIdentityProxyModel::data(
                proxyIndex.siblingAtColumn(pTrackTableModel->fieldIndex(
                        ColumnCache::COLUMN_LIBRARYTABLE_COLOR)),
                Qt::DisplayRole)
                        .toDouble());
    }
    case CoverArt: {
        QString location;
        if (pTrackTableModel != nullptr) {
            location = QIdentityProxyModel::data(
                    proxyIndex.siblingAtColumn(pTrackTableModel->fieldIndex(
                            ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION)),
                    Qt::DisplayRole)
                               .toString();
        } else if (pTrackModel != nullptr) {
            auto pTrack = pTrackModel->getTrack(
                    QIdentityProxyModel::mapToSource(proxyIndex));
            location = pTrack->getCoverInfo().coverLocation;
        }
        if (location.isEmpty()) {
            return {};
        }

        return AsyncImageProvider::trackLocationToCoverArtUrl(location);
    }
    case FileURL: {
        if (pTrackModel == nullptr) {
            return {};
        }
        return pTrackModel->getTrackUrl(QIdentityProxyModel::mapToSource(proxyIndex));
    }
    case Delegate:
        return QVariant::fromValue(pColumn->delegate());
        break;
    }
    if (pColumn->columnIdx() < 0) {
        // Use proxyIndex.column()
        return QIdentityProxyModel::data(proxyIndex, role);
    }
    return QIdentityProxyModel::data(
            proxyIndex.siblingAtColumn(pTrackTableModel != nullptr
                            ? pTrackTableModel->fieldIndex(
                                      static_cast<ColumnCache::Column>(
                                              pColumn->columnIdx()))
                            : pColumn->columnIdx()),
            role);
}

int QmlLibraryTrackListModel::columnCount(const QModelIndex& parent) const {
    VERIFY_OR_DEBUG_ASSERT(static_cast<QmlLibraryTrackListModel*>(
                                   parent.internalPointer()) != this) {
        return 0;
    }
    return m_columns.size();
}

QVariant QmlLibraryTrackListModel::headerData(
        int section, Qt::Orientation orientation, int role) const {
    VERIFY_OR_DEBUG_ASSERT(section >= 0 || section < m_columns.size()) {
        return {};
    }
    // TODO role
    return m_columns[section]->label();
}

QHash<int, QByteArray> QmlLibraryTrackListModel::roleNames() const {
    return kRoleNames;
}

QUrl QmlLibraryTrackListModel::getUrl(int row) const {
    auto* const pTrackModel = dynamic_cast<TrackModel*>(sourceModel());

    if (pTrackModel == nullptr) {
        // TODO search for column with role
        return {};
    }
    return pTrackModel->getTrackUrl(sourceModel()->index(row, 0));
}

QmlTrackProxy* QmlLibraryTrackListModel::getTrack(int row) const {
    auto* const pTrackModel = dynamic_cast<TrackModel*>(sourceModel());

    if (pTrackModel == nullptr) {
        // TODO search for column with role
        return {};
    }
    return make_qml_owned<QmlTrackProxy>(pTrackModel->getTrack(sourceModel()->index(row, 0)));
}

TrackModel::Capabilities QmlLibraryTrackListModel::getCapabilities() const {
    auto* const pTrackModel = dynamic_cast<TrackModel*>(sourceModel());

    if (pTrackModel != nullptr) {
        return pTrackModel->getCapabilities();
    }
    return TrackModel::Capability::None;
}
bool QmlLibraryTrackListModel::hasCapabilities(TrackModel::Capabilities caps) const {
    return (getCapabilities() & caps) == caps;
}
void QmlLibraryTrackListModel::sort(int column, Qt::SortOrder order) {
    VERIFY_OR_DEBUG_ASSERT(column >= 0 || column < m_columns.size()) {
        return;
    }
    const auto& pColumn = m_columns[column];
    emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(),
            QAbstractItemModel::VerticalSortHint);
    if (pColumn->columnIdx() < 0) {
        // Use proxyIndex.column()
        return sourceModel()->sort(column, order);
    }
    auto* const pTrackTableModel = qobject_cast<BaseTrackTableModel*>(sourceModel());
    sourceModel()->sort(pTrackTableModel != nullptr
                    ? pTrackTableModel->fieldIndex(
                              static_cast<ColumnCache::Column>(
                                      pColumn->columnIdx()))
                    : pColumn->columnIdx(),
            order);
    emit layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
}

} // namespace qml
} // namespace mixxx
