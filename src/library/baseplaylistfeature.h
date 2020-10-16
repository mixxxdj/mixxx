#pragma once

#include <QAction>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QPair>
#include <QSet>
#include <QString>
#include <QUrl>

#include "library/basetracksetfeature.h"
#include "library/dao/playlistdao.h"
#include "track/track_decl.h"

class WLibrary;
class KeyboardEventFilter;
class PlaylistTableModel;
class TrackCollectionManager;
class TreeItem;

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

  public slots:
    void activateChild(const QModelIndex& index) override;
    virtual void activatePlaylist(int playlistId);
    virtual void htmlLinkClicked(const QUrl& link);

    virtual void slotPlaylistTableChanged(int playlistId) = 0;
    virtual void slotPlaylistContentChanged(QSet<int> playlistIds) = 0;
    virtual void slotPlaylistTableRenamed(int playlistId, QString newName) = 0;
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

    virtual QModelIndex constructChildModel(int selected_id);
    virtual void updateChildModel(int selected_id);
    virtual void clearChildModel();
    virtual QList<IdAndLabel> createPlaylistLabels() = 0;
    virtual QString fetchPlaylistLabel(int playlistId) = 0;
    virtual void decorateChild(TreeItem* pChild, int playlistId) = 0;
    virtual void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);

    int playlistIdFromIndex(QModelIndex index);
    // Get the QModelIndex of a playlist based on its id.  Returns QModelIndex()
    // on failure.
    QModelIndex indexFromPlaylistId(int playlistId);

    PlaylistDAO& m_playlistDao;
    TreeItemModel m_childModel;
    QModelIndex m_lastRightClickedIndex;

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

  private slots:
    void slotTrackSelected(TrackPointer pTrack);
    void slotResetSelectedTrack();

  private:
    void initActions();
    virtual QString getRootViewHtml() const = 0;

    TrackPointer m_pSelectedTrack;

    QSet<int> m_playlistsSelectedTrackIsIn;
};
