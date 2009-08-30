// playlistfeature.h
// Created 8/17/09 by RJ Ryan (rryan@mit.edu)

#ifndef PLAYLISTFEATURE_H
#define PLAYLISTFEATURE_H

#include <QAction>
#include <QList>

#include "library/libraryfeature.h"

class PlaylistTableModel;
class TrackCollection;

class PlaylistFeature : public LibraryFeature {
    Q_OBJECT
public:
    PlaylistFeature(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~PlaylistFeature();
    QVariant title();
    QIcon getIcon();
    int numChildren();
    QVariant child(int n);
    bool dropAccept(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(const QModelIndex& index, QUrl url);                             
public slots:
    void activate();
    void activateChild(int n);
    void onRightClick(const QPoint& globalPos, QModelIndex index);
    void onClick(QModelIndex index);
  
    void slotCreatePlaylist();
    void slotDeletePlaylist();
 private:
    QList<QString> playlists;
    PlaylistTableModel* m_pPlaylistTableModel;   
    TrackCollection* m_pTrackCollection;
    QAction *m_pCreatePlaylistAction;
    QAction *m_pDeletePlaylistAction;
    QModelIndex m_lastRightClickedIndex;
};

#endif /* PLAYLISTFEATURE_H */
