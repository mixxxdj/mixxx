#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QPointer>
#include <QStringListModel>
#include <QtConcurrentRun>
#include <QtSql>
#include <atomic>

#include "library/baseexternallibraryfeature.h"
#include "library/itunes/itunesimporter.h"
#include "library/itunes/itunespathmapping.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"
#include "library/treeitemmodel.h"
#include "util/parented_ptr.h"

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
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activate(bool forceReload);
    void activateChild(const QModelIndex& index) override;
    void onRightClick(const QPoint& globalPos) override;
    void onTrackCollectionLoaded();

  private:
    std::unique_ptr<BaseSqlTableModel> createPlaylistModelForPlaylist(
            const QString& playlist) override;
    static QString getiTunesMusicPath();
    std::unique_ptr<ITunesImporter> makeImporter();
    // returns the invisible rootItem for the sidebar model
    TreeItem* importLibrary();
    void guessMusicLibraryMountpoint(QXmlStreamReader& xml);
    void parseTracks(QXmlStreamReader& xml);
    void parseTrack(QXmlStreamReader& xml, QSqlQuery& query);
    TreeItem* parsePlaylists(QXmlStreamReader &xml);
    void parsePlaylist(QXmlStreamReader& xml, QSqlQuery& query1,
                       QSqlQuery &query2, TreeItem*);
    void clearTable(const QString& table_name);
    bool readNextStartElement(QXmlStreamReader& xml);

    /// Presents an 'open file' dialog for selecting an iTunes library XML and
    /// returns the file path.
    QString showOpenDialog();

    bool isMacOSImporterUsed();

    BaseExternalTrackModel* m_pITunesTrackModel;
    BaseExternalPlaylistModel* m_pITunesPlaylistModel;
    parented_ptr<TreeItemModel> m_pSidebarModel;
    QStringList m_playlists;
    // a new DB connection for the worker thread
    QSqlDatabase m_database;
    std::atomic<bool> m_cancelImport;
    bool m_isActivated;

    /// The path to the iTunes library XML file. If this is empty and the
    /// user is running macOS, we will use the `ITunesMacOSImporter` that
    /// uses the native `iTunesLibrary` framework to import the user's
    /// library. In all other cases, the `ITunesXMLImporter` will be used
    /// to parse the given file.
    QString m_dbfile;

    QFutureWatcher<TreeItem*> m_future_watcher;
    QFuture<TreeItem*> m_future;
    QString m_title;

    ITunesPathMapping m_pathMapping;

    QSharedPointer<BaseTrackCache> m_trackSource;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
