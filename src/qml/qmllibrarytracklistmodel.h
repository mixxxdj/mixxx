#pragma once
#include <QIdentityProxyModel>
#include <QQmlEngine>

#include "library/trackmodel.h"
#include "qml/qmllibrarytracklistcolumn.h"
#include "qml/qmltrackproxy.h"

namespace mixxx {
namespace qml {

class QmlLibraryTrackListModel : public QIdentityProxyModel {
    Q_OBJECT
    QML_NAMED_ELEMENT(LibraryTrackListModel)
    Q_PROPERTY(QQmlListProperty<QmlLibraryTrackListColumn> columns READ columns FINAL)
    QML_UNCREATABLE("Only accessible via Mixxx.Library")

  public:
    enum Roles {
        Track = Qt::UserRole,
        FileURL,
        CoverArt,
        Delegate
    };
    Q_ENUM(Roles);

    // FIXME Remove the enum duplication with the `Capability` in `trackmodel.h`
    enum class Capability {
        None = 0u,
        Reorder = 1u << 0u,
        ReceiveDrops = 1u << 1u,
        AddToTrackSet = 1u << 2u,
        AddToAutoDJ = 1u << 3u,
        Locked = 1u << 4u,
        EditMetadata = 1u << 5u,
        LoadToDeck = 1u << 6u,
        LoadToSampler = 1u << 7u,
        LoadToPreviewDeck = 1u << 8u,
        Remove = 1u << 9u,
        ResetPlayed = 1u << 10u,
        Hide = 1u << 11u,
        Unhide = 1u << 12u,
        Purge = 1u << 13u,
        RemovePlaylist = 1u << 14u,
        RemoveCrate = 1u << 15u,
        RemoveFromDisk = 1u << 16u,
        Analyze = 1u << 17u,
        Properties = 1u << 18u,
        Sorting = 1u << 19u,
    };
    Q_ENUM(Capability)

    QmlLibraryTrackListModel(const QList<QmlLibraryTrackListColumn*>& librarySource,
            QAbstractItemModel* pModel,
            QObject* pParent = nullptr);
    ~QmlLibraryTrackListModel() = default;

    QQmlListProperty<QmlLibraryTrackListColumn> columns() {
        return {this,
                &m_columns,
                parent_qlist_append,
                parent_qlist_count,
                parent_qlist_at,
                parent_qlist_clear};
    }

    QVariant data(const QModelIndex& index, int role) const override;
    int columnCount(const QModelIndex& index = QModelIndex()) const override;
    Q_INVOKABLE QUrl getUrl(int row) const;
    Q_INVOKABLE mixxx::qml::QmlTrackProxy* getTrack(int row) const;
    Q_INVOKABLE TrackModel::Capabilities getCapabilities() const;
    Q_INVOKABLE bool hasCapabilities(TrackModel::Capabilities caps) const;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant headerData(int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;
    Q_INVOKABLE void sort(int column, Qt::SortOrder order) override;

  private:
    std::vector<parented_ptr<QmlLibraryTrackListColumn>> m_columns;

    static void parent_qlist_append(
            QQmlListProperty<QmlLibraryTrackListColumn>* p,
            QmlLibraryTrackListColumn* v) {
        reinterpret_cast<std::vector<parented_ptr<QmlLibraryTrackListColumn>>*>(
                p->data)
                ->emplace_back(v);
    }
    static qsizetype parent_qlist_count(QQmlListProperty<QmlLibraryTrackListColumn>* p) {
        return reinterpret_cast<
                std::vector<parented_ptr<QmlLibraryTrackListColumn>>*>(p->data)
                ->size();
    }
    static QmlLibraryTrackListColumn* parent_qlist_at(
            QQmlListProperty<QmlLibraryTrackListColumn>* p, qsizetype idx) {
        return reinterpret_cast<
                std::vector<parented_ptr<QmlLibraryTrackListColumn>>*>(p->data)
                ->at(idx)
                .get();
    }
    static void parent_qlist_clear(QQmlListProperty<QmlLibraryTrackListColumn>* p) {
        return reinterpret_cast<
                std::vector<parented_ptr<QmlLibraryTrackListColumn>>*>(p->data)
                ->clear();
    }
};

} // namespace qml
} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::qml::QmlLibraryTrackListModel*)
