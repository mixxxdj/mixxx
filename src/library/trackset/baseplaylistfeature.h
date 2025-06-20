#pragma once

#include <QModelIndex>
#include <QPointer>
#include <QSet>
#include <QString>

#include "library/dao/playlistdao.h"
#include "library/trackset/basetracksetfeature.h"
#include "track/trackid.h"
#include "util/parented_ptr.h"

class WLibrary;
class KeyboardEventFilter;
class PlaylistTableModel;
class TreeItem;
class WLibrarySidebar;
class QAction;
class QUrl;

class BasePlaylistFeature : public BaseTrackSetFeature {
    Q_OBJECT

  public:
    BasePlaylistFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            PlaylistTableModel* pModel,
            const QString& rootViewName,
            const QString& iconName,
            const QString& countsDurationTableName,
            bool keepHiddenTracks = false);
    ~BasePlaylistFeature() override = default;

    TreeItemModel* sidebarModel() const override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;
    void selectPlaylistInSidebar(int playlistId, bool select = true);
    int getSiblingPlaylistIdOf(QModelIndex& start);

  public slots:
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

#ifdef __ENGINEPRIME__
  signals:
    void exportAllPlaylists();
    void exportPlaylist(int playlistId);
#endif

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
    // Get the QModelIndex of a playlist based on its id.  Returns QModelIndex()
    // on failure.
    QModelIndex indexFromPlaylistId(int playlistId);
    bool isChildIndexSelectedInSidebar(const QModelIndex& index);

    QString createPlaylistLabel(const QString& name, int count, int duration) const;

    PlaylistDAO& m_playlistDao;
    QModelIndex m_lastClickedIndex;
    QModelIndex m_lastRightClickedIndex;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
    QPointer<WLibrary> m_pLibraryWidget;

    parented_ptr<QAction> m_pCreatePlaylistAction;
    parented_ptr<QAction> m_pDeletePlaylistAction;
    parented_ptr<QAction> m_pAddToAutoDJAction;
    parented_ptr<QAction> m_pAddToAutoDJTopAction;
    parented_ptr<QAction> m_pAddToAutoDJReplaceAction;
    parented_ptr<QAction> m_pRenamePlaylistAction;
    parented_ptr<QAction> m_pLockPlaylistAction;
    parented_ptr<QAction> m_pImportPlaylistAction;
    parented_ptr<QAction> m_pCreateImportPlaylistAction;
    parented_ptr<QAction> m_pExportPlaylistAction;
    parented_ptr<QAction> m_pExportTrackFilesAction;
    parented_ptr<QAction> m_pDuplicatePlaylistAction;
    parented_ptr<QAction> m_pAnalyzePlaylistAction;
#ifdef __ENGINEPRIME__
    parented_ptr<QAction> m_pExportAllPlaylistsToEngineAction;
    parented_ptr<QAction> m_pExportPlaylistToEngineAction;
#endif

    PlaylistTableModel* m_pPlaylistTableModel;
    QSet<int> m_playlistIdsOfSelectedTrack;
    const QString m_countsDurationTableName;
    TrackId m_selectedTrackId;

  private slots:
    void slotTrackSelected(TrackId trackId);
    void slotResetSelectedTrack();

  private:
    void initActions();
    void connectPlaylistDAO();
    virtual QString getRootViewHtml() const = 0;
    void markTreeItem(TreeItem* pTreeItem);
    QString fetchPlaylistLabel(int playlistId);


    const bool m_keepHiddenTracks;
};
