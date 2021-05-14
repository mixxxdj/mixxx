#pragma once

#include <QAction>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QPair>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QUrl>

#include "library/dao/playlistdao.h"
#include "library/trackset/basetracksetfeature.h"
#include "track/track_decl.h"

class WLibrary;
class KeyboardEventFilter;
class PlaylistTableModel;
class TrackCollectionManager;
class TreeItem;
class WLibrarySidebar;

constexpr int kInvalidPlaylistId = -1;

class BasePlaylistFeature : public BaseTrackSetFeature {
    Q_OBJECT

  public:
    BasePlaylistFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            PlaylistTableModel* pModel,
            const QString& rootViewName);
    ~BasePlaylistFeature() override = default;

    TreeItemModel* getChildModel() override;

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
    virtual void slotPlaylistTableRenamed(int playlistId, const QString& newName) = 0;
    virtual void slotPlaylistContentChanged(QSet<int> playlistIds) = 0;
    void slotCreatePlaylist();

  protected slots:
    void slotDeletePlaylist();
    void slotDuplicatePlaylist();
    void slotAddToAutoDJ();
    void slotAddToAutoDJTop();
    void slotAddToAutoDJReplace();
    void slotRenamePlaylist();
    void slotTogglePlaylistLock();
    void slotImportPlaylist();
    void slotImportPlaylistFile(const QString& playlist_file);
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

    virtual void updateChildModel(int selected_id);
    virtual void clearChildModel();
    virtual QString fetchPlaylistLabel(int playlistId) = 0;
    virtual void decorateChild(TreeItem* pChild, int playlistId) = 0;
    virtual void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);

    int playlistIdFromIndex(const QModelIndex& index);
    // Get the QModelIndex of a playlist based on its id.  Returns QModelIndex()
    // on failure.
    QModelIndex indexFromPlaylistId(int playlistId);

    PlaylistDAO& m_playlistDao;
    QModelIndex m_lastRightClickedIndex;
    QPointer<WLibrarySidebar> m_pSidebarWidget;

    QAction* m_pCreatePlaylistAction;
    QAction* m_pDeletePlaylistAction;
    QAction* m_pAddToAutoDJAction;
    QAction* m_pAddToAutoDJTopAction;
    QAction* m_pAddToAutoDJReplaceAction;
    QAction* m_pRenamePlaylistAction;
    QAction* m_pLockPlaylistAction;
    QAction* m_pImportPlaylistAction;
    QAction* m_pCreateImportPlaylistAction;
    QAction* m_pExportPlaylistAction;
    QAction* m_pExportTrackFilesAction;
    QAction* m_pDuplicatePlaylistAction;
    QAction* m_pAnalyzePlaylistAction;

    PlaylistTableModel* m_pPlaylistTableModel;
    QSet<int> m_playlistIdsOfSelectedTrack;

  private slots:
    void slotTrackSelected(TrackPointer pTrack);
    void slotResetSelectedTrack();

  private:
    void initActions();
    virtual QString getRootViewHtml() const = 0;
    void markTreeItem(TreeItem* pTreeItem);

    TrackPointer m_pSelectedTrack;
};
