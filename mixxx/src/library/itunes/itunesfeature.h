#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QPointer>
#include <atomic>

#include "library/baseexternallibraryfeature.h"
#include "util/parented_ptr.h"

class BaseExternalTrackModel;
class BaseExternalPlaylistModel;
class WLibrarySidebar;
class ITunesImporter;
class BaseTrackCache;

class ITunesFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
 public:
    ITunesFeature(Library* pLibrary, UserSettingsPointer pConfig);
    virtual ~ITunesFeature();
    static bool isSupported();

    QVariant title() override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

    // This is called from the ITunesXMLImporter thread and generally threadsafe
    bool isImportCanceled() const {
        return m_cancelImport.load();
    }

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
    void clearTable(const QString& table_name);

    /// Presents an 'open file' dialog for selecting an iTunes library XML and
    /// returns the file path.
    QString showOpenDialog();

    bool isNativeImporterUsed();

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

    QSharedPointer<BaseTrackCache> m_trackSource;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
};
