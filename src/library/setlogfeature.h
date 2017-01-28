// setlogfeature.h

#ifndef SETLOGFEATURE_H
#define SETLOGFEATURE_H

#include <QLinkedList>
#include <QSqlTableModel>
#include <QAction>

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

  public slots:
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void slotJoinWithPrevious();
    void slotGetNewPlaylist();

  protected:
    void buildPlaylistList();
    void decorateChild(TreeItem *pChild, int playlist_id);

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
};

#endif // SETLOGFEATURE_H
