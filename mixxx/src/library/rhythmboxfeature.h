// rhythmboxfeature.h 
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef RHYTHMBOXFEATURE_H
#define RHYTHMBOXFEATURE_H

#include "library/libraryfeature.h"

class RhythmboxPlaylistModel;
class RhythmboxTrackModel;

class RhythmboxFeature : public LibraryFeature {
 Q_OBJECT
 public:
    RhythmboxFeature(QObject* parent = NULL);
    virtual ~RhythmboxFeature();

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
    RhythmboxTrackModel* m_pRhythmboxTrackModel;
    RhythmboxPlaylistModel* m_pRhythmboxPlaylistModel;
};

#endif /* RHYTHMBOXFEATURE_H */
