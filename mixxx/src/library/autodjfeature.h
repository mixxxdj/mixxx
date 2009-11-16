// AutoDJfeature.h
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef AUTODJFEATURE_H
#define AUTODJFEATURE_H

#include <QStringListModel>

#include "library/libraryfeature.h"
#include "library/dao/playlistdao.h"

class PlaylistTableModel;
class ProxyTrackModel;
class TrackCollection;

class AutoDJFeature : public LibraryFeature {
    Q_OBJECT
    public:
    AutoDJFeature(QObject* parent,
                        TrackCollection* pTrackCollection);
    virtual ~AutoDJFeature();

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
    //PlaylistTableModel* m_pAutoDJTableModel;
    //ProxyTrackModel* m_pAutoDJTableModelProxy;
    QStringListModel m_childModel;
};


#endif /* AUTODJFEATURE_H */
