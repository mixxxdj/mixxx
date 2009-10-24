// playlistfeature.h
// Created 8/17/09 by RJ Ryan (rryan@mit.edu)

#ifndef PLAYLISTFEATURE_H
#define PLAYLISTFEATURE_H

#include <QSqlTableModel>
#include <QAction>
#include <QList>

#include "library/libraryfeature.h"

class PlaylistTableModel;
class ProxyTrackModel;
class TrackCollection;

class PlaylistFeature : public LibraryFeature {
    Q_OBJECT
public:
    PlaylistFeature(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~PlaylistFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    QAbstractItemModel* getChildModel();

    void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget);
public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

    void slotCreatePlaylist();
    void slotDeletePlaylist();

 private:
    PlaylistTableModel* m_pPlaylistTableModel;
    ProxyTrackModel* m_pPlaylistModelProxy;
    TrackCollection* m_pTrackCollection;
    QAction *m_pCreatePlaylistAction;
    QAction *m_pDeletePlaylistAction;
    QSqlTableModel m_playlistTableModel;
    QModelIndex m_lastRightClickedIndex;
};

#endif /* PLAYLISTFEATURE_H */
