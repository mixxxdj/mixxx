// playlistfeature.h
// Created 8/17/09 by RJ Ryan (rryan@mit.edu)

#ifndef PLAYLISTFEATURE_H
#define PLAYLISTFEATURE_H

#include <QList>

#include "library/libraryfeature.h"

class PlaylistFeature : public LibraryFeature {
    Q_OBJECT
public:
    PlaylistFeature(QObject* parent = NULL);
    virtual ~PlaylistFeature();
    QVariant title();
    QIcon getIcon();
    int numChildren();
    QVariant child(int n);
                         
public slots:
    void activate();
    void activateChild(int n);
    void onRightClick(QModelIndex index);
    void onClick(QModelIndex index);
    
 private:
    QList<QString> playlists;
    
};

#endif /* PLAYLISTFEATURE_H */
