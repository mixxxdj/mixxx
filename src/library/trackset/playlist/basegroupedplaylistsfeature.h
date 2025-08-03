#pragma once

#include <QModelIndex>
#include <QPointer>
#include <QSet>
#include <QString>

#include "library/dao/playlistdao.h"
#include "library/playlisttablemodel.h"
#include "library/trackset/basetracksetfeature.h"
#include "track/trackid.h"

class WLibrary;
class KeyboardEventFilter;
class GroupedPlaylistsTableModel;
class TreeItem;
class WLibrarySidebar;
class QAction;
class QUrl;

class PlaylistSummary;

class BaseGroupedPlaylistsFeature : public BaseTrackSetFeature {
    Q_OBJECT

  public:
    BaseGroupedPlaylistsFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            GroupedPlaylistsTableModel* pModel,
            const QString& rootViewName,
            const QString& iconName,
            const QString& countsDurationTableName,
            bool keepHiddenTracks = false);
    ~BaseGroupedPlaylistsFeature() override = default;

    TreeItemModel* sidebarModel() const override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;
    void selectPlaylistInSidebar(int playlistId, bool select = true);
    int getSiblingPlaylistIdOf(QModelIndex& start);

  public slots:
    QModelIndex rebuildChildModel(int selectedPlaylistId);

    void activateChild(const QModelIndex& index) override;
    virtual void activatePlaylist(int playlistId);
    virtual void htmlLinkClicked(const QUrl& link);

    virtual void slotPlaylistTableChanged(int playlistId) = 0;
    void slotPlaylistTableChangedAndSelect(int playlistId) {
        slotPlaylistTableChanged(playlistId);
        selectPlaylistInSidebar(playlistId);
    };
    void slotPlaylistTableChangedAndScrollTo(int playlistId) {
        slotPlaylistTableChanged(playlistId);
        selectPlaylistInSidebar(playlistId, false);
    };
    virtual void slotPlaylistContentOrLockChanged(const QSet<int>& playlistIds) = 0;
    virtual void slotPlaylistTableRenamed(int playlistId, const QString& newName) = 0;
    void slotCreatePlaylist();
    void renameItem(const QModelIndex& index) override;
    void deleteItem(const QModelIndex& index) override;

  protected slots:
    virtual void slotDeletePlaylist();
    void slotDuplicatePlaylist();
    void slotAddToAutoDJ();
    void slotAddToAutoDJTop();
    void slotAddToAutoDJReplace();
    void slotRenamePlaylist();
    void slotTogglePlaylistLock();
    void slotImportPlaylist();
    void slotImportPlaylistFile(const QString& playlistFile, int playlistId);
    void slotCreateImportPlaylist();
    void slotExportPlaylist();
    // Copy all of the tracks in a playlist to a new directory.
    void slotExportTrackFiles();
    void slotAnalyzePlaylist();

  protected:
    const QIcon m_lockedPlaylistIcon;
    struct IdAndLabel {
        int id;
        QString label;
    };

    virtual void updateChildModel(const QSet<int>& playlistIds);
    virtual void clearChildModel();

    /// borrows pChild which must not be null, TODO: use gsl::not_null
    virtual void decorateChild(TreeItem* pChild, int playlistId) = 0;
    virtual void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);

    int playlistIdFromIndex(const QModelIndex& index) const;
    // PlaylistId playlistIdFromIndex(const QModelIndex& index) const;
    //  Get the QModelIndex of a playlist based on its id.  Returns QModelIndex()
    //  on failure.
    QModelIndex indexFromPlaylistId(int playlistId);
    bool isChildIndexSelectedInSidebar(const QModelIndex& index);

    QString createPlaylistLabel(const QString& name, int count, int duration) const;

    PlaylistDAO& m_playlistDao;
    QModelIndex m_lastClickedIndex;
    QModelIndex m_lastRightClickedIndex;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
    QPointer<WLibrary> m_pLibraryWidget;

    QAction* m_pCreateGroupedPlaylistsAction;
    QAction* m_pDeleteGroupedPlaylistsAction;
    QAction* m_pAddToAutoDJAction;
    QAction* m_pAddToAutoDJTopAction;
    QAction* m_pAddToAutoDJReplaceAction;
    QAction* m_pRenameGroupedPlaylistsAction;
    QAction* m_pLockGroupedPlaylistsAction;
    QAction* m_pImportGroupedPlaylistsAction;
    QAction* m_pCreateImportGroupedPlaylistsAction;
    QAction* m_pExportGroupedPlaylistsAction;
    QAction* m_pExportTrackFilesAction;
    QAction* m_pDuplicateGroupedPlaylistsAction;
    QAction* m_pAnalyzeGroupedPlaylistsAction;

    GroupedPlaylistsTableModel* m_pGroupedPlaylistsTableModel;
    QSet<int> m_playlistIdsOfSelectedTrack;
    const QString m_countsDurationTableName;
    TrackId m_selectedTrackId;

  private slots:
    void slotTrackSelected(TrackId trackId);
    void slotResetSelectedTrack();

  private:
    // Stores the id of a playlist in the sidebar that is adjacent to the playlist(playlistId).
    void storePrevSiblingPlaylistId(int playlistId);
    //  Can be used to restore a similar selection after the sidebar model was rebuilt.
    int m_prevSiblingPlaylist;

    TrackCollection* const m_pTrackCollection;

    QString fullPathFromIndex(const QModelIndex& index) const;
    QString groupNameFromIndex(const QModelIndex& index) const;
    void updateFullPathRecursive(TreeItem* pItem, const QString& parentPath);

    void initActions();
    void connectPlaylistDAO();
    virtual QString getRootViewHtml() const = 0;
    void markTreeItem(TreeItem* pTreeItem);
    QString fetchPlaylistLabel(int playlistId);

    const bool m_keepHiddenTracks;
};
