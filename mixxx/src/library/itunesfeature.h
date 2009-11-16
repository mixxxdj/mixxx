// rhythmboxfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef ITUNESFEATURE_H
#define ITUNESBOXFEATURE_H

#include <QStringListModel>

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
    ProxyTrackModel* m_pTrackModelProxy;
    //ITunesPlaylistModel* m_pITunesPlaylistModel;
    QStringListModel m_childModel;
};

#endif /* ITUNESFEATURE_H */
