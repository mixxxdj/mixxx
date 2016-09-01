// browsefeature.h
// Created 9/8/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BROWSEFEATURE_H
#define BROWSEFEATURE_H

#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QObject>
#include <QVariant>
#include <QHash>
#include <QIcon>
#include <QModelIndex>
#include <QPoint>
#include <QString>

#include "preferences/usersettings.h"
#include "library/browse/browsetablemodel.h"
#include "library/browse/foldertreemodel.h"
#include "library/libraryfeature.h"
#include "library/proxytrackmodel.h"

#define QUICK_LINK_NODE "::mixxx_quick_lnk_node::"
#define DEVICE_NODE "::mixxx_device_node::"

class TrackCollection;
class WLibraryStack;

class BrowseFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BrowseFeature(UserSettingsPointer pConfig,
                  Library* pLibrary,
                  QObject* parent,
                  TrackCollection* pTrackCollection,
                  RecordingManager* pRec);
    virtual ~BrowseFeature();

    QVariant title();
    QIcon getIcon();

    QWidget* createPaneWidget(KeyboardEventFilter*pKeyboard, int paneId) override;

    TreeItemModel* getChildModel();

  public slots:
    void slotAddQuickLink();
    void slotRemoveQuickLink();
    void slotAddToLibrary();
    void activate();
    void activateChild(const QModelIndex& index);
    void onRightClickChild(const QPoint& globalPos, QModelIndex index);
    void onLazyChildExpandation(const QModelIndex& index);
    void slotLibraryScanStarted();
    void slotLibraryScanFinished();

  signals:
    void setRootIndex(const QModelIndex&);
    void requestAddDir(QString);
    void scanLibrary();

  private:
    QString getRootViewHtml() const;
    QString extractNameFromPath(QString spath);
    QStringList getDefaultQuickLinks() const;
    void saveQuickLinks();
    void loadQuickLinks();

    BrowseTableModel m_browseModel;
    ProxyTrackModel m_proxyModel;
    TrackCollection* m_pTrackCollection;
    FolderTreeModel m_childModel;
    QAction* m_pAddQuickLinkAction;
    QAction* m_pRemoveQuickLinkAction;
    QAction* m_pAddtoLibraryAction;
    TreeItem* m_pLastRightClickedItem;
    TreeItem* m_pQuickLinkItem;
    QStringList m_quickLinkList;
    
    QHash<int, QPointer<WLibraryStack> > m_panes;
    QHash<int, int> m_idBrowse;
    QHash<int, int> m_idTable;
};

#endif // BROWSEFEATURE_H
