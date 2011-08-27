// playlistfeature.h
// Created 8/17/09 by RJ Ryan (rryan@mit.edu)

#ifndef PLAYLISTFEATURE_H
#define PLAYLISTFEATURE_H

#include <QSqlTableModel>
#include <QAction>
#include <QList>

#include "library/libraryfeature.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "treeitemmodel.h"
#include "configobject.h"


class PlaylistTableModel;
class TrackCollection;

class PlaylistFeature : public LibraryFeature {
    Q_OBJECT
public:
    PlaylistFeature(QObject* parent, TrackCollection* pTrackCollection, ConfigObject<ConfigValue>* pConfig);
    virtual ~PlaylistFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    TreeItemModel* getChildModel();

    void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);
  signals:
    void showPage(const QUrl& page);

public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void onLazyChildExpandation(const QModelIndex& index);

    void slotCreatePlaylist();
    void slotDeletePlaylist();
    void slotAddToAutoDJ();
    void slotAddToAutoDJTop();
    void slotRenamePlaylist();
    void slotTogglePlaylistLock();
    void slotImportPlaylist();
    void slotExportPlaylist();

    void slotPlaylistTableChanged(int playlistId);

 private:
    QModelIndex constructChildModel(int selected_id);
    void clearChildModel();
    void addToAutoDJ(bool bTop);

    PlaylistTableModel* m_pPlaylistTableModel;
    PlaylistDAO &m_playlistDao;
    TrackDAO &m_trackDao;
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
    ConfigObject<ConfigValue>* m_pConfig;
};

#endif /* PLAYLISTFEATURE_H */
