#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <QPointer>
#include <QString>

#include "library/libraryfeature.h"
#include "preferences/usersettings.h"
#include "util/parented_ptr.h"

#define QUICK_LINK_NODE "::mixxx_quick_lnk_node::"
#define DEVICE_NODE "::mixxx_device_node::"

class BrowseTableModel;
class BrowseLibraryTableModel;
class ProxyTrackModel;
class FolderTreeModel;
class Library;
class RecordingManager;
class TrackCollection;
class TrackModel;
class WLibrarySidebar;

class BrowseFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BrowseFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            RecordingManager* pRecordingManager);
    ~BrowseFeature() override;

    QVariant title() override;

    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget) override;

    TreeItemModel* sidebarModel() const override;

    void releaseBrowseThread();

  public slots:
    void slotAddQuickLink();
    void slotRemoveQuickLink();
    void slotAddToLibrary();
    void slotRefreshDirectoryTree();
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void onLazyChildExpandation(const QModelIndex& index) override;
    void slotLibraryScanStarted();
    void slotLibraryScanFinished();
    void invalidateRightClickIndex();

  signals:
    void setRootIndex(const QModelIndex&);
    void requestAddDir(const QString&);
    void scanLibrary();

  private slots:
    void slotLibraryDirectoriesChanged();
    void onSymLinkMapUpdated();

  private:
    QString getRootViewHtml() const;
    QString extractNameFromPath(const QString& spath);
    bool isPathWatched(const QString& path) const;
    QString maybeUnResolveSymlink(const QString& path) const;

    QMap<QString, QString> updateSymlinkList(const QList<mixxx::FileInfo>& rootDirs);
    void slotUpdateAllTreeItemsIsWatchedPath();
    void updateItemIsWatchedPathRecursively(TreeItem* pItem);

    QStringList getDefaultQuickLinks() const;
    std::vector<std::unique_ptr<TreeItem>> createRemovableDevices() const;
    std::vector<std::unique_ptr<TreeItem>> getChildDirectoryItems(
            const QString& path,
            bool isWatched) const;
    std::unique_ptr<TreeItem> createPathTreeItem(const QString& name,
            const QString& path,
            bool parentIsWatched = false) const;
    void saveQuickLinks();
    void loadQuickLinks();
    QString getLastRightClickedPath() const;

    QString getCurrentSearch() const;

    TrackCollection* const m_pTrackCollection;

    parented_ptr<BrowseTableModel> m_pBrowseModel;
    std::unique_ptr<ProxyTrackModel> m_pProxyModel;
    parented_ptr<BrowseLibraryTableModel> m_pLibraryTableModel;
    TrackModel* m_pCurrentTrackModel;
    parented_ptr<FolderTreeModel> m_pSidebarModel;
    parented_ptr<QAction> m_pAddQuickLinkAction;
    parented_ptr<QAction> m_pRemoveQuickLinkAction;
    parented_ptr<QAction> m_pAddtoLibraryAction;
    parented_ptr<QAction> m_pRefreshDirTreeAction;

    // Caution: Make sure this is reset whenever the library tree is updated,
    // so that the internalPointer() does not become dangling
    QModelIndex m_lastRightClickedIndex;
    TreeItem* m_pQuickLinkItem;
    QStringList m_quickLinkList;
    QPointer<WLibrarySidebar> m_pSidebarWidget;

    QMap<QString, QString> m_trackDirSymlinksMap;

    QFutureWatcher<QMap<QString, QString>> m_future_watcher;
    QFuture<QMap<QString, QString>> m_future;
    QMutex m_mutex;
};
