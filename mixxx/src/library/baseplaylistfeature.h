#ifndef BASEPLAYLISTFEATURE_H
#define BASEPLAYLISTFEATURE_H

#include <QSqlTableModel>
#include <QAction>

#include "library/libraryfeature.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"

class WLibrarySidebar;
class WLibrary;
class MixxxKeyboard;
class PlaylistTableModel;
class TrackCollection;

class BasePlaylistFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BasePlaylistFeature(QObject* parent,
                        ConfigObject<ConfigValue>* pConfig,
                        TrackCollection* pTrackCollection,
                        QString rootViewName, QString rootViewUrl);
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
    virtual bool dropAccept(QUrl url);
    virtual bool dragMoveAccept(QUrl url);
    virtual void onLazyChildExpandation(const QModelIndex& index);

    void slotCreatePlaylist();

  protected slots:
    void slotDeletePlaylist();
    void slotAddToAutoDJ();
    void slotAddToAutoDJTop();
    void slotRenamePlaylist();
    void slotTogglePlaylistLock();
    void slotImportPlaylist();
    void slotExportPlaylist();
    virtual void slotPlaylistTableChanged(int playlistId) = 0;

  protected:
    virtual QModelIndex constructChildModel(int selected_id) = 0;
    virtual void clearChildModel();
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
    QSqlTableModel m_playlistTableModel;
    QModelIndex m_lastRightClickedIndex;
    TreeItemModel m_childModel;

  private:
    QString m_rootViewName;
    QString m_rootViewUrl;
};

#endif /* BASEPLAYLISTFEATURE_H */
