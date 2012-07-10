// itunesfeaturefeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef ITUNESFEATURE_H
#define ITUNESFEATURE_H

#include <QStringListModel>
#include <QtSql>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "library/baseexternallibraryfeature.h"
#include "library/trackcollection.h"
#include "library/treeitemmodel.h"
#include "library/treeitem.h"

//class ITunesPlaylistModel;
class ITunesTrackModel;
class ITunesPlaylistModel;


class ITunesFeature : public BaseExternalLibraryFeature {
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

    TreeItemModel* getChildModel();

  public slots:
    void activate();
    void activate(bool forceReload);
    void activateChild(const QModelIndex& index);
    virtual void onRightClick(const QPoint& globalPos);
    void onLazyChildExpandation(const QModelIndex& index);
    void onTrackCollectionLoaded();

  private:
    virtual BaseSqlTableModel* getPlaylistModelForPlaylist(QString playlist);
    static QString getiTunesMusicPath();
    //returns the invisible rootItem for the sidebar model
    TreeItem* importLibrary();
    void parseTracks(QXmlStreamReader &xml);
    void parseTrack(QXmlStreamReader &xml, QSqlQuery &query);
    TreeItem* parsePlaylists(QXmlStreamReader &xml);
    void parsePlaylist(QXmlStreamReader &xml, QSqlQuery &query1, QSqlQuery &query2, TreeItem*);
    void clearTable(QString table_name);
    bool readNextStartElement(QXmlStreamReader& xml);

    ITunesTrackModel* m_pITunesTrackModel;
    ITunesPlaylistModel* m_pITunesPlaylistModel;
    TreeItemModel m_childModel;
    QStringList m_playlists;
    TrackCollection* m_pTrackCollection;
    //a new DB connection for the worker thread
    QSqlDatabase m_database;
    bool m_cancelImport;
    bool m_isActivated;
    QString m_dbfile;

    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;

    QString m_dbItunesRoot;
    QString m_mixxxItunesRoot;

    static const QString ITDB_PATH_KEY;
};

#endif /* ITUNESFEATURE_H */
