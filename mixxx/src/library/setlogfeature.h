// setlogfeature.h
// Created 8/17/09 by RJ Ryan (rryan@mit.edu)

#ifndef SETLOGFEATURE_H
#define SETLOGFEATURE_H

#include <QSqlTableModel>
#include <QAction>
#include <QList>

#include "library/libraryfeature.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "treeitemmodel.h"
#include "configobject.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"



class PlaylistTableModel;
class TrackCollection;

class SetlogFeature : public LibraryFeature {
    Q_OBJECT
public:
    SetlogFeature(QObject* parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection);
    virtual ~SetlogFeature();

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

    void slotDeletePlaylist();
    void slotAddToAutoDJ();
    void slotAddToAutoDJTop();
    void slotRenamePlaylist();
    void slotTogglePlaylistLock();
    void slotExportPlaylist();
    void slotJoinWithPrevious();

    void slotPositionChanged(double /*value*/);
    void slotPlaylistTableChanged(int playlistId);

 private:
    QModelIndex constructChildModel(int selected_id);
    void clearChildModel();
    void addToAutoDJ(bool bTop);

    TrackCollection* m_pTrackCollection;
    PlaylistTableModel* m_pPlaylistTableModel;
    PlaylistDAO &m_playlistDao;
    TrackDAO &m_trackDao;
    QAction *m_pDeletePlaylistAction;
    QAction *m_pAddToAutoDJAction;
    QAction *m_pAddToAutoDJTopAction;
    QAction *m_pRenamePlaylistAction;
    QAction *m_pLockPlaylistAction;
    QAction *m_pExportPlaylistAction;
    QAction *m_pJoinWithPreviousAction;
    ControlObjectThreadMain* m_pCOPlayPos1;
    ControlObjectThreadMain* m_pCOPlayPos2;
    QModelIndex m_lastRightClickedIndex;
    TreeItemModel m_childModel;
    ConfigObject<ConfigValue>* m_pConfig;
    QSqlTableModel m_playlistTableModel;
    const static QString m_sSetlogViewName;
    int m_playlistId;
    int m_oldTrackIdPlayer[2];
    //int m_oldTrackPlayer2;
    //int m_oldCurrendPlayingTrack;
};

#endif /* SETLOGFEATURE_H */
