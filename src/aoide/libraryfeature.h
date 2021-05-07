#pragma once

#include <QAction>
#include <QJsonArray>
#include <QPointer>

#include "aoide/json/playlist.h"
#include "aoide/tracksearchoverlayfilter.h"
#include "library/libraryfeature.h"
#include "library/treeitemmodel.h"
#include "util/parented_ptr.h"
#include "util/qt.h"

class Library;

namespace aoide {

class Subsystem;
class ListCollectedPlaylistsTask;
class CollectionListModel;
class TrackTableModel;

class LibraryFeature : public ::LibraryFeature {
    Q_OBJECT

  public:
    LibraryFeature(
            Library* library,
            UserSettingsPointer settings,
            Subsystem* subsystem);
    ~LibraryFeature() override;

    QVariant title() override;

    void bindLibraryWidget(
            WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(
            WLibrarySidebar* sidebarWidget) override;
    TreeItemModel* sidebarModel() const override;

    bool hasTrackTable() override {
        return true;
    }

    bool dragMoveAcceptChild(
            const QModelIndex& index,
            const QUrl& url) override;
    bool dropAcceptChild(
            const QModelIndex& index,
            const QList<QUrl>& urls,
            QObject* pSource) override;

  public slots:
    void activate() override;
    void activateChild(
            const QModelIndex& index) override;
    void onRightClick(
            const QPoint& globalPos) override;
    void onRightClickChild(
            const QPoint& globalPos,
            const QModelIndex& index) override;

  private slots:
    void slotTrackSearchOverlayFilter();

    void slotLoadPreparedQueries();
    void slotSavePreparedQueries();

    void slotRefreshQueryResults();

    void slotReloadPlaylists();

    void slotRefreshPlaylistEntries();

    void slotCreatePlaylist();
    void slotPlaylistCreated(
            const json::PlaylistEntity& playlistEntity);
    void slotDeletePlaylist();
    void slotPlaylistDeleted(
            const QString& playlistUid);

    void slotPlaylistAppendTrackEntriesByUrlSucceeded(
            const json::PlaylistWithEntriesSummaryEntity& playlistEntity,
            const QList<QUrl>& unresolvedTrackUrls);

    void slotListCollectedPlaylistsTaskSucceeded(
            const QVector<json::PlaylistWithEntriesSummaryEntity>& result);

    void reactivateChild();

  private:
    const QString m_title;

    const QIcon m_preparedQueriesIcon;
    const QIcon m_playlistsIcon;

    QAction* const m_trackSearchOverlayFilterAction;

    QAction* const m_loadPreparedQueriesAction;
    QAction* const m_savePreparedQueriesAction;

    QAction* const m_refreshQueryResultsAction;

    QAction* const m_reloadPlaylistsAction;
    QAction* const m_createPlaylistAction;
    QAction* const m_deletePlaylistAction;

    QAction* const m_refreshPlaylistEntriesAction;

    const QPointer<Subsystem> m_subsystem;

    const parented_ptr<CollectionListModel> m_collectionListModel;

    const parented_ptr<TrackTableModel> m_trackTableModel;

    parented_ptr<TreeItemModel> m_sidebarModel;

    QJsonArray m_preparedQueries;

    TrackSearchOverlayFilter m_trackSearchOverlayFilter;

    QVector<json::PlaylistWithEntriesSummaryEntity> m_playlistEntities;

    bool reloadPreparedQueries(const QString& filePath);
    bool reloadCollectedPlaylists();

    void rebuildChildModel();

    QJsonObject preparedQueryAt(
            const QModelIndex& index) const;
    json::PlaylistWithEntriesSummaryEntity playlistAt(
            const QModelIndex& index) const;

    QModelIndex m_activeChildIndex;

    QString m_previousSearch;

    mixxx::SafeQPointer<ListCollectedPlaylistsTask> m_pendingListCollectedPlaylistsTask;
};

} // namespace aoide
