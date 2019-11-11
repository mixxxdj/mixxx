// setlogfeature.h

#ifndef SETLOGFEATURE_H
#define SETLOGFEATURE_H

#include <QLinkedList>
#include <QSqlTableModel>
#include <QAction>
#include <QPointer>

#include "library/baseplaylistfeature.h"
#include "preferences/usersettings.h"

class TrackCollection;
class TreeItem;
class WLibrarySidebar;

class SetlogFeature : public BasePlaylistFeature {
    Q_OBJECT
public:
    SetlogFeature(QObject* parent, UserSettingsPointer pConfig,
                  TrackCollection* pTrackCollection);
    virtual ~SetlogFeature();

    QVariant title() override;
    QIcon getIcon() override;

    void bindLibraryWidget(WLibrary* libraryWidget,
                           KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

  public slots:
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, QModelIndex index) override;
    void slotJoinWithPrevious();
    void slotGetNewPlaylist();

  protected:
    QList<BasePlaylistFeature::IdAndLabel> createPlaylistLabels() override;
    QString fetchPlaylistLabel(int playlistId) override;
    void decorateChild(TreeItem *pChild, int playlistId) override;

  private slots:
    void slotPlayingTrackChanged(TrackPointer currentPlayingTrack);
    void slotPlaylistTableChanged(int playlistId) override;
    void slotPlaylistContentChanged(int playlistId) override;
    void slotPlaylistTableRenamed(int playlistId, QString a_strName) override;

  private:
    QString getRootViewHtml() const override;

    QLinkedList<TrackId> m_recentTracks;
    QAction* m_pJoinWithPreviousAction;
    QAction* m_pGetNewPlaylist;
    int m_playlistId;
    WLibrary* m_libraryWidget;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
    QIcon m_icon;
};

#endif // SETLOGFEATURE_H
