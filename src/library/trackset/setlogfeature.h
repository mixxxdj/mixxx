#pragma once

#include <QPointer>

#include "library/trackset/baseplaylistfeature.h"
#include "preferences/usersettings.h"

class Library;
class QAction;

class SetlogFeature : public BasePlaylistFeature {
    Q_OBJECT

  public:
    SetlogFeature(Library* pLibrary,
            UserSettingsPointer pConfig);
    virtual ~SetlogFeature();

    QVariant title() override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void activatePlaylist(int playlistId) override;
    bool isItemDataUnique(const QVariant& data) const override {
        return data != QVariant(m_yearNodeId);
    }

  public slots:
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

  protected:
    QModelIndex constructChildModel(int selectedId);
    void decorateChild(TreeItem* pChild, int playlistId) override;

  private slots:
    void slotPlayingTrackChanged(TrackPointer currentPlayingTrack);
    void slotPlaylistTableChanged(
            int playlistId,
            PlaylistDAO::HiddenType type) override;
    void slotPlaylistContentOrLockChanged(const QSet<int>& playlistIds) override;
    void slotPlaylistTableRenamed(int playlistId, const QString& newName) override;
    void slotDeleteAllUnlockedChildPlaylists();

  private:
    void deleteAllUnlockedPlaylistsWithFewerTracks();
    void lockOrUnlockAllChildPlaylists(bool lock);
    QString getRootViewHtml() const override;

    std::list<TrackId> m_recentTracks;
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
