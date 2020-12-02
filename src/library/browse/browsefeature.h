// browsefeature.h
// Created 9/8/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BROWSEFEATURE_H
#define BROWSEFEATURE_H

#include <QByteArrayData>
#include <QIcon>
#include <QModelIndex>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QVariant>

#include "library/browse/browsetablemodel.h"
#include "library/browse/foldertreemodel.h"
#include "library/libraryfeature.h"
#include "library/proxytrackmodel.h"
#include "preferences/usersettings.h"

class KeyboardEventFilter;
class QAction;
class QModelIndex;
class QObject;
class QPoint;
class RecordingManager;
class TreeItem;
class TreeItemModel;
class WLibrary;

#define QUICK_LINK_NODE "::mixxx_quick_lnk_node::"
#define DEVICE_NODE "::mixxx_device_node::"

class Library;
class TrackCollection;
class WLibrarySidebar;

class BrowseFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BrowseFeature(Library* pLibrary,
            UserSettingsPointer pConfig,
            RecordingManager* pRecordingManager);
    virtual ~BrowseFeature();

    QVariant title();
    QIcon getIcon();

    void bindLibraryWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* keyboard);
    void bindSidebarWidget(WLibrarySidebar* pSidebarWidget);

    TreeItemModel* getChildModel();

  public slots:
    void slotAddQuickLink();
    void slotRemoveQuickLink();
    void slotAddToLibrary();
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index);
    void onLazyChildExpandation(const QModelIndex& index);
    void slotLibraryScanStarted();
    void slotLibraryScanFinished();

  signals:
    void setRootIndex(const QModelIndex&);
    void requestAddDir(const QString&);
    void scanLibrary();

  private:
    QString getRootViewHtml() const;
    QString extractNameFromPath(const QString& spath);
    QStringList getDefaultQuickLinks() const;
    void saveQuickLinks();
    void loadQuickLinks();

    TrackCollection* const m_pTrackCollection;

    BrowseTableModel m_browseModel;
    ProxyTrackModel m_proxyModel;
    FolderTreeModel m_childModel;
    QAction* m_pAddQuickLinkAction;
    QAction* m_pRemoveQuickLinkAction;
    QAction* m_pAddtoLibraryAction;
    TreeItem* m_pLastRightClickedItem;
    TreeItem* m_pQuickLinkItem;
    QStringList m_quickLinkList;
    QIcon m_icon;
    QPointer<WLibrarySidebar> m_pSidebarWidget;
};

#endif // BROWSEFEATURE_H
