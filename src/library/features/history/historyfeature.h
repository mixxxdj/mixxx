// setlogfeature.h

#ifndef SETLOGFEATURE_H
#define SETLOGFEATURE_H

#include <QLinkedList>

#include "library/features/baseplaylist/baseplaylistfeature.h"
#include "library/library.h"

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
    QString getIconPath() override;
    QString getSettingsName() const override;
    void decorateChild(TreeItem *pChild, int playlist_id) override;
    QPointer<TreeItemModel> getChildModel() override;

  public slots:
    void onRightClick(const QPoint&) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotJoinWithNext();
    void slotGetNewPlaylist();

  protected:
    parented_ptr<QWidget> createInnerSidebarWidget(KeyboardEventFilter*,
                                                   QWidget* parent) override;

    const TreeItemModel* getConstChildModel() const override;
    void buildPlaylistList() override;
    QModelIndex constructChildModel(int selected_id);
    parented_ptr<PlaylistTableModel> constructTableModel() override;
    QSet<int> playlistIdsFromIndex(const QModelIndex &index) const override;
    QModelIndex indexFromPlaylistId(int playlistId) const override;

  private slots:
    void slotPlayingTrackChanged(TrackPointer currentPlayingTrack);
    void slotPlaylistTableChanged(int playlistId) override;
    void slotPlaylistContentChanged(int playlistId) override;
    void slotPlaylistTableRenamed(int playlistId, QString a_strName) override;

  private:
    QString getRootViewHtml() const override;

    Library *m_pLibrary;
    QPointer<WLibrarySidebar> m_pSidebar;
    QLinkedList<TrackId> m_recentTracks;
    parented_ptr<QAction> m_pJoinWithNextAction;
    parented_ptr<QAction> m_pGetNewPlaylist;
    std::unique_ptr<HistoryTreeModel> m_pHistoryTreeModel;
    int m_playlistId;
};

#endif // SETLOGFEATURE_H
