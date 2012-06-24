// browsefeature.h
// Created 9/8/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BROWSEFEATURE_H
#define BROWSEFEATURE_H

#include <QStringListModel>
#include <QSortFilterProxyModel>

#include "configobject.h"
#include "library/browse/browsetablemodel.h"
#include "library/browse/foldertreemodel.h"
#include "library/libraryfeature.h"
#include "library/proxytrackmodel.h"

#define QUICK_LINK_NODE "::mixxx_quick_lnk_node::"
#define DEVICE_NODE "::mixxx_device_node::"

class TrackCollection;

class BrowseFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BrowseFeature(QObject* parent,
                  ConfigObject<ConfigValue>* pConfig,
                  TrackCollection* pTrackCollection,
                  RecordingManager* pRec);
    virtual ~BrowseFeature();

    QVariant title();
    QIcon getIcon();

    bool dropAccept(QUrl url);
    bool dropAcceptChild(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(QUrl url);
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url);

    void bindWidget(WLibrarySidebar* sidebarWidget,
                    WLibrary* libraryWidget,
                    MixxxKeyboard* keyboard);

    TreeItemModel* getChildModel();

  public slots:
    void slotAddQuickLink();
    void slotRemoveQuickLink();
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClick(const QPoint& globalPos);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);

    void onLazyChildExpandation(const QModelIndex& index);

  signals:
    void setRootIndex(const QModelIndex&);

  private:
    QString getRootViewHtml() const;
    QString extractNameFromPath(QString spath);
    QStringList getDefaultQuickLinks() const;
    void saveQuickLinks();
    void loadQuickLinks();

    ConfigObject<ConfigValue>* m_pConfig;
    BrowseTableModel m_browseModel;
    ProxyTrackModel m_proxyModel;
    TrackCollection* m_pTrackCollection;
    FolderTreeModel m_childModel;
    QAction* m_pAddQuickLinkAction;
    QAction* m_pRemoveQuickLinkAction;
    TreeItem* m_pLastRightClickedItem;
    TreeItem* m_pQuickLinkItem;
    QStringList m_quickLinkList;
};

#endif /* BROWSEFEATURE_H */
