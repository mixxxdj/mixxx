// rhythmboxfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef RHYTHMBOXFEATURE_H
#define RHYTHMBOXFEATURE_H

#include <QStringListModel>
#include <QtSql>
#include <QXmlStreamReader>

#include "library/libraryfeature.h"
#include "treeitemmodel.h"
#include "library/rhythmboxtrackmodel.h"
#include "library/rhythmboxplaylistmodel.h"
#include "library/trackcollection.h"



class RhythmboxFeature : public LibraryFeature {
 Q_OBJECT
 public:
    RhythmboxFeature(QObject* parent, TrackCollection*);
    virtual ~RhythmboxFeature();
    static bool isSupported();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    TreeItemModel* getChildModel();
    bool importMusicCollection();
    bool importPlaylists();

public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

private:
    RhythmboxTrackModel* m_pRhythmboxTrackModel;
    RhythmboxPlaylistModel* m_pRhythmboxPlaylistModel;
    TrackCollection* m_pTrackCollection;
    QSqlDatabase &m_database;
    bool m_isActivated;
    
    TreeItemModel m_childModel;
};

#endif /* RHYTHMBOXFEATURE_H */
