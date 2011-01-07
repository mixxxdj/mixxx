// rhythmboxfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef ITUNESFEATURE_H
#define ITUNESBOXFEATURE_H

#include <QStringListModel>
#include <QtSql>

#include "library/libraryfeature.h"
#include "library/trackcollection.h"

//class ITunesPlaylistModel;
class ITunesTrackModel;
class ITunesPlaylistModel;


class ITunesFeature : public LibraryFeature {
 Q_OBJECT
 public:
    ITunesFeature(QObject* parent, TrackCollection* pTrackCollection);
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
    void activate(bool forceReload, bool askToLoad = true);
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

  private:
    static QString getiTunesMusicPath();
    bool importLibrary(QString file);
    void parseTracks(QXmlStreamReader &xml);
    void parseTrack(QXmlStreamReader &xml, QSqlQuery &query);
    void parsePlaylists(QXmlStreamReader &xml);
    void parsePlaylist(QXmlStreamReader &xml, QSqlQuery &query1, QSqlQuery &query2);
    void clearTable(QString table_name);

    ITunesTrackModel* m_pITunesTrackModel;
    ITunesPlaylistModel* m_pITunesPlaylistModel;
    QStringListModel m_childModel;
    QStringList m_playlists;
    TrackCollection* m_pTrackCollection;
    QSqlDatabase &m_database;
    bool m_isActivated;

    static const QString ITDB_PATH_KEY;
};

#endif /* ITUNESFEATURE_H */
