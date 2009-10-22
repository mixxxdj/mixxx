// rhythmboxfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef ITUNESFEATURE_H
#define ITUNESBOXFEATURE_H

#include "library/libraryfeature.h"

//class ITunesPlaylistModel;
class ITunesTrackModel;
class ProxyTrackModel;

class ITunesFeature : public LibraryFeature {
 Q_OBJECT
 public:
    ITunesFeature(QObject* parent = NULL);
    virtual ~ITunesFeature();

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

private:
    ITunesTrackModel* m_pITunesTrackModel;
    ProxyTrackModel* m_pTrackModelProxy;
    //ITunesPlaylistModel* m_pITunesPlaylistModel;
};

#endif /* ITUNESFEATURE_H */
