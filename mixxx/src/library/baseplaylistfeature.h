#ifndef BASEPLAYLISTFEATURE_H
#define BASEPLAYLISTFEATURE_H

#include <QAction>

#include "library/libraryfeature.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"

class WLibrarySidebar;
class WLibrary;
class MixxxKeyboard;
class PlaylistTableModel;
class TrackCollection;
class TreeItem;

class BasePlaylistFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BasePlaylistFeature(QObject* parent,
                        ConfigObject<ConfigValue>* pConfig,
                        TrackCollection* pTrackCollection,
                        QString rootViewName);
    virtual ~BasePlaylistFeature();

    TreeItemModel* getChildModel();

    void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

  signals:
    void showPage(const QUrl& page);

  public slots:
    virtual void activate();
    virtual void activateChild(const QModelIndex& index);
    virtual bool dropAccept(QList<QUrl> urls);
    virtual bool dragMoveAccept(QUrl url);
    virtual void onLazyChildExpandation(const QModelIndex& index);
    virtual void htmlLinkClicked(const QUrl & link);

    virtual void slotPlaylistTableChanged(int playlistId) = 0;
    void slotCreatePlaylist();

  protected slots:
    void slotDeletePlaylist();
    void slotAddToAutoDJ();
    void slotAddToAutoDJTop();
    void slotRenamePlaylist();
    void slotTogglePlaylistLock();
    void slotImportPlaylist();
    void slotExportPlaylist();

  protected:
    virtual QModelIndex constructChildModel(int selected_id);
    virtual void clearChildModel();
    virtual void buildPlaylistList() = 0;
    virtual void decorateChild(TreeItem *pChild, int playlist_id) = 0;
    virtual void addToAutoDJ(bool bTop);

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    PlaylistDAO &m_playlistDao;
    TrackDAO &m_trackDao;
    PlaylistTableModel* m_pPlaylistTableModel;
    QAction *m_pCreatePlaylistAction;
    QAction *m_pDeletePlaylistAction;
    QAction *m_pAddToAutoDJAction;
    QAction *m_pAddToAutoDJTopAction;
    QAction *m_pRenamePlaylistAction;
    QAction *m_pLockPlaylistAction;
    QAction *m_pImportPlaylistAction;
    QAction *m_pExportPlaylistAction;
    QList<QPair<int, QString> > m_playlistList;
    QModelIndex m_lastRightClickedIndex;
    TreeItemModel m_childModel;

  private:
    virtual QString getRootViewHtml() const = 0;

    QString m_rootViewName;
};

#endif /* BASEPLAYLISTFEATURE_H */
