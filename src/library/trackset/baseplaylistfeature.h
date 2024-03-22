#pragma once

#include <QModelIndex>
#include <QPointer>
#include <QSet>
#include <QString>

#include "library/dao/playlistdao.h"
#include "library/trackset/basetracksetfeature.h"
#include "track/trackid.h"

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
            const QString& iconName);
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
    virtual QString fetchPlaylistLabel(int playlistId) = 0;

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
    void slotTrackSelected(TrackId trackId);
    void slotResetSelectedTrack();

  private:
    void initActions();
    void connectPlaylistDAO();
    virtual QString getRootViewHtml() const = 0;
    void markTreeItem(TreeItem* pTreeItem);

    TrackId m_selectedTrackId;
};
