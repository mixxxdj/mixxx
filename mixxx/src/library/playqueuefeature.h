// playqueuefeature.h
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef PLAYQUEUEFEATURE_H
#define PLAYQUEUEFEATURE_H

#include <QStringListModel>

#include "library/libraryfeature.h"
#include "library/dao/playlistdao.h"

class PlaylistTableModel;
class ProxyTrackModel;
class TrackCollection;

class PlayQueueFeature : public LibraryFeature {
    Q_OBJECT
    public:
    PlayQueueFeature(QObject* parent,
                        TrackCollection* pTrackCollection);
    virtual ~PlayQueueFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    QAbstractItemModel* getChildModel();

public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

private:
    TrackCollection *m_pTrackCollection;
    PlaylistDAO& m_playlistDao;
    PlaylistTableModel* m_pPlayQueueTableModel;
    ProxyTrackModel* m_pPlayQueueTableModelProxy;
    QStringListModel m_childModel;
};


#endif /* PLAYQUEUEFEATURE_H */
