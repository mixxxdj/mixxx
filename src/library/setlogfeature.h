#pragma once

#include <QAction>
#include <QPointer>
#include <QSqlTableModel>

#include "library/baseplaylistfeature.h"
#include "preferences/usersettings.h"

class SetlogFeature : public BasePlaylistFeature {
    Q_OBJECT
  public:
    SetlogFeature(Library* pLibrary,
            UserSettingsPointer pConfig);
    virtual ~SetlogFeature();

    QVariant title() override;
    QIcon getIcon() override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

  public slots:
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotJoinWithPrevious();
    void slotGetNewPlaylist();

  protected:
    QList<BasePlaylistFeature::IdAndLabel> createPlaylistLabels() override;
    QString fetchPlaylistLabel(int playlistId) override;
    void decorateChild(TreeItem* pChild, int playlistId) override;

  private slots:
    void slotPlayingTrackChanged(TrackPointer currentPlayingTrack);
    void slotPlaylistTableChanged(int playlistId) override;
    void slotPlaylistContentChanged(QSet<int> playlistIds) override;
    void slotPlaylistTableRenamed(int playlistId, const QString& newName) override;

  private:
    void reloadChildModel(int playlistId);
    QString getRootViewHtml() const override;

    std::list<TrackId> m_recentTracks;
    QAction* m_pJoinWithPreviousAction;
    QAction* m_pGetNewPlaylist;
    int m_playlistId;
    WLibrary* m_libraryWidget;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
    const QIcon m_icon;
};
