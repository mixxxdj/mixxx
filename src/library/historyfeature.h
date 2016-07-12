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

class HistoryFeature : public BasePlaylistFeature {
    Q_OBJECT
public:
    HistoryFeature(UserSettingsPointer pConfig,
                  Library* pLibrary,
                  QObject* parent, 
                  TrackCollection* pTrackCollection);
    virtual ~HistoryFeature();

    QVariant title();
    QIcon getIcon();

  public slots:
    void onRightClick(const QPoint&);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void slotJoinWithPrevious();
    void slotGetNewPlaylist();

  protected:
    void buildPlaylistList();
    void decorateChild(TreeItem *pChild, int playlist_id);
    PlaylistTableModel* constructTableModel();

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
