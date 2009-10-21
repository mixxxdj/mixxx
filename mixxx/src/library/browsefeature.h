// browsefeature.h
// Created 9/8/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BROWSEFEATURE_H
#define BROWSEFEATURE_H

#include <QSortFilterProxyModel>
#include <QFileSystemModel>

#include "configobject.h"
#include "library/browsefilter.h"
#include "library/libraryfeature.h"

class TrackCollection;

class BrowseFeature : public LibraryFeature {
    Q_OBJECT
  public:
    BrowseFeature(QObject* parent,
                  ConfigObject<ConfigValue>* pConfig,
                  TrackCollection* pTrackCollection);
    virtual ~BrowseFeature();
    QVariant title();
    QIcon getIcon();
    int numChildren();
    QVariant child(int n);
    bool dropAccept(const QModelIndex& index, QUrl url);
    bool dragMoveAccept(const QModelIndex& index, QUrl url);
    virtual void bindWidget(WLibrarySidebar* sidebarWidget,
                            WLibrary* libraryWidget);

  public slots:
    void activate();
    void activateChild(int n);
    void onRightClick(const QPoint& globalPos, QModelIndex index);
    void onClick(QModelIndex index);

    // Called when a file in browse view is selected to be loaded into a player.
    void loadToPlayer(const QModelIndex& index, int player);
    // Called when a file in the browse view is activated.
    void onFileActivate(const QModelIndex& index);
    void searchStarting();
    void search(const QString&);
    void searchCleared();
  signals:
    void setRootIndex(const QModelIndex&);
  private:
    ConfigObject<ConfigValue>* m_pConfig;
    QFileSystemModel m_fileSystemModel;
    BrowseFilter m_proxyModel;
    TrackCollection* m_pTrackCollection;
    QString m_currentSearch;
};

#endif /* BROWSEFEATURE_H */
