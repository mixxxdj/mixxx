// setlogfeature.h

#ifndef SETLOGFEATURE_H
#define SETLOGFEATURE_H

#include <QLinkedList>
#include <QSqlTableModel>
#include <QAction>
#include <QMenu>

#include "library/baseplaylistfeature.h"
#include "preferences/usersettings.h"

class TrackCollection;
class TreeItem;

class SetlogFeature : public BasePlaylistFeature {
    Q_OBJECT
public:
    SetlogFeature(QObject* parent, UserSettingsPointer pConfig,
                  TrackCollection* pTrackCollection);
    virtual ~SetlogFeature();

    QVariant title();
    QIcon getIcon();

    virtual void bindWidget(WLibrary* libraryWidget,
                            KeyboardEventFilter* keyboard);
    virtual void bindSidebarWidget(WLibrarySidebar* pSidebarWidget);

  public slots:
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void slotJoinWithPrevious();
    void slotGetNewPlaylist();

  protected:
    QList<BasePlaylistFeature::IdAndLabel> createPlaylistLabels() override;
    QString fetchPlaylistLabel(int playlistId) override;
    void decorateChild(TreeItem *pChild, int playlistId) override;

  private slots:
    void slotPlayingTrackChanged(TrackPointer currentPlayingTrack);
    void slotPlaylistTableChanged(int playlistId);
    void slotPlaylistContentChanged(int playlistId);
    void slotPlaylistTableRenamed(int playlistId, QString a_strName);

  private:
    QString getRootViewHtml() const;

    QLinkedList<TrackId> m_recentTracks;
    QAction* m_pJoinWithPreviousAction;
    QAction* m_pGetNewPlaylist;
    int m_playlistId;
    WLibrary* m_libraryWidget;
    WLibrarySidebar* m_pSidebarWidget;
    QIcon m_icon;
    QMenu *m_pMenu;
};

#endif // SETLOGFEATURE_H
