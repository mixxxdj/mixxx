// rhythmboxfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef ITUNESFEATURE_H
#define ITUNESBOXFEATURE_H

#include <QStringListModel>

#include "library/libraryfeature.h"

//class ITunesPlaylistModel;
class ITunesTrackModel;
class ITunesPlaylistModel;
class ProxyTrackModel;

class ITunesFeature : public LibraryFeature {
 Q_OBJECT
 public:
    ITunesFeature(QObject* parent = NULL);
    virtual ~ITunesFeature();
    static bool isSupported();

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
    ITunesTrackModel* m_pITunesTrackModel;
    ITunesPlaylistModel* m_pITunesPlaylistModel;
    ProxyTrackModel* m_pTrackModelProxy;
    ProxyTrackModel* m_pPlaylistModelProxy;
    QStringListModel m_childModel;
};

#endif /* ITUNESFEATURE_H */
