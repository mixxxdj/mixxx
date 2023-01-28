#pragma once

#include <QAction>
#include <QPointer>
#include <QSqlTableModel>

#include "library/trackset/baseplaylistfeature.h"
#include "preferences/usersettings.h"

class Library;

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

  public slots:
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotJoinWithPrevious();
    void slotLockAllChildPlaylists();
    void slotUnlockAllChildPlaylists();
    void slotDeletePlaylist() override;
    void slotGetNewPlaylist();
    void activate() override;

  protected:
    QModelIndex constructChildModel(int selectedId);
    QString fetchPlaylistLabel(int playlistId) override;
    void decorateChild(TreeItem* pChild, int playlistId) override;

  private slots:
    void slotPlayingTrackChanged(TrackPointer currentPlayingTrack);
    void slotPlaylistTableChanged(int playlistId) override;
    void slotPlaylistContentChanged(QSet<int> playlistIds) override;
    void slotPlaylistTableLockChanged(int playlistId) override;
    void slotPlaylistTableRenamed(int playlistId, const QString& newName) override;
    void slotDeleteAllChildPlaylists();

  private:
    void deleteAllUnlockedPlaylistsWithFewerTracks();
    void lockOrUnlockAllChildPlaylists(bool lock);
    QString getRootViewHtml() const override;

    std::list<TrackId> m_recentTracks;
    QAction* m_pJoinWithPreviousAction;
    QAction* m_pStartNewPlaylist;
    QAction* m_pLockAllChildPlaylists;
    QAction* m_pUnlockAllChildPlaylists;
    QAction* m_pDeleteAllChildPlaylists;

    int m_playlistId;
    int m_placeholderId;

    QPointer<WLibrary> m_libraryWidget;
    Library* m_pLibrary;
    UserSettingsPointer m_pConfig;
};
