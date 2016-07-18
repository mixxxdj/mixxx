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
class HistoryTreeModel;

class HistoryFeature : public BasePlaylistFeature {
    Q_OBJECT
public:
    HistoryFeature(UserSettingsPointer pConfig,
                  Library* pLibrary,
                  QObject* parent, 
                  TrackCollection* pTrackCollection);
    virtual ~HistoryFeature();

    QVariant title() override;
    QIcon getIcon() override;

  public slots:
    void onRightClick(const QPoint&) override;
    void onRightClickChild(const QPoint& globalPos, QModelIndex index) override;
    void slotJoinWithPrevious();
    void slotGetNewPlaylist();

  protected:
    void buildPlaylistList() override;
    void decorateChild(TreeItem *pChild, int playlist_id) override;
    PlaylistTableModel* constructTableModel() override;
    QSet<int> playlistIdsFromIndex(const QModelIndex &index) const override;

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
    HistoryTreeModel* m_pHistoryTreeModel;
    int m_playlistId;
};

#endif // SETLOGFEATURE_H
