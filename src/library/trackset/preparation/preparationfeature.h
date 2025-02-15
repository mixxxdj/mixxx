#pragma once

#include <QPointer>

#include "library/trackset/baseplaylistfeature.h"
#include "preferences/usersettings.h"

class Library;
class QAction;

class PreparationFeature : public BasePlaylistFeature {
    Q_OBJECT

  public:
    PreparationFeature(Library* pLibrary,
            UserSettingsPointer pConfig);
    virtual ~PreparationFeature();

    QVariant title() override;

    void bindLibraryWidget(WLibrary* plibraryWidget,
            KeyboardEventFilter* pkeyboard);
    void activatePlaylist(int playlistId) override;

  public slots:
    void slotAddLoadedTrackToPreparation(const QString& group,
            TrackPointer loadedTrack,
            TrackPointer pOldTrack);
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotJoinWithPrevious();
    void slotMarkAllTracksPlayed();
    void slotLockAllChildPlaylists();
    void slotUnlockAllChildPlaylists();
    void slotDeletePlaylist() override;
    void slotGetNewPlaylist();
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void slotShowInLibraryWindow();

  protected:
    QModelIndex constructChildModel(int selectedId);
    void decorateChild(TreeItem* pChild, int playlistId) override;

  private slots:
    void slotPlaylistTableChanged(int playlistId) override;
    void slotPlaylistContentOrLockChanged(const QSet<int>& playlistIds) override;
    void slotPlaylistTableRenamed(int playlistId, const QString& newName) override;
    void slotDeleteAllUnlockedChildPlaylists();

  private:
    void deleteAllUnlockedPlaylistsWithFewerTracks();
    void lockOrUnlockAllChildPlaylists(bool lock);
    QString getRootViewHtml() const override;

    std::list<TrackId> m_recentTracks;
    QAction* m_pShowTrackModelInLibraryWindowAction;
    QAction* m_pJoinWithPreviousAction;
    QAction* m_pMarkTracksPlayedAction;
    QAction* m_pStartNewPlaylist;
    QAction* m_pLockAllChildPlaylists;
    QAction* m_pUnlockAllChildPlaylists;
    QAction* m_pDeleteAllChildPlaylists;

    int m_currentPlaylistId;
    int m_yearNodeId;
    Library* m_pLibrary;
    UserSettingsPointer m_pConfig;
};
