// itunesfeaturefeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef ITUNESFEATURE_H
#define ITUNESFEATURE_H

#include <QStringListModel>
#include <QtSql>
#include <QFuture>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QPointer>

#include "library/baseexternallibraryfeature.h"
#include "library/trackcollection.h"
#include "library/treeitemmodel.h"
#include "library/treeitem.h"

class BaseExternalTrackModel;
class BaseExternalPlaylistModel;
class WLibrarySidebar;

class ITunesFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
 public:
    ITunesFeature(Library* pLibrary, UserSettingsPointer pConfig);
    virtual ~ITunesFeature();
    static bool isSupported();

    QVariant title() override;
    QIcon getIcon() override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* getChildModel() override;

  public slots:
    void activate() override;
    void activate(bool forceReload);
    void activateChild(const QModelIndex& index) override;
    void onRightClick(const QPoint& globalPos) override;
    void onTrackCollectionLoaded();

  private:
    BaseSqlTableModel* getPlaylistModelForPlaylist(QString playlist) override;
    static QString getiTunesMusicPath();
    // returns the invisible rootItem for the sidebar model
    TreeItem* importLibrary();
    void guessMusicLibraryMountpoint(QXmlStreamReader& xml);
    void parseTracks(QXmlStreamReader& xml);
    void parseTrack(QXmlStreamReader& xml, QSqlQuery& query);
    TreeItem* parsePlaylists(QXmlStreamReader &xml);
    void parsePlaylist(QXmlStreamReader& xml, QSqlQuery& query1,
                       QSqlQuery &query2, TreeItem*);
    void clearTable(QString table_name);
    bool readNextStartElement(QXmlStreamReader& xml);

    BaseExternalTrackModel* m_pITunesTrackModel;
    BaseExternalPlaylistModel* m_pITunesPlaylistModel;
    TreeItemModel m_childModel;
    QStringList m_playlists;
    // a new DB connection for the worker thread
    QSqlDatabase m_database;
    bool m_cancelImport;
    bool m_isActivated;
    QString m_dbfile;

    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;

    QString m_dbItunesRoot;
    QString m_mixxxItunesRoot;

    QSharedPointer<BaseTrackCache> m_trackSource;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
    QIcon m_icon;
};

#endif // ITUNESFEATURE_H
