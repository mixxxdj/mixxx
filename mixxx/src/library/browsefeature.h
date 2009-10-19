// browsefeature.h
// Created 9/8/2009 by RJ Ryan (rryan@mit.edu)

#ifndef BROWSEFEATURE_H
#define BROWSEFEATURE_H

#include <QFileSystemModel>

#include "configobject.h"
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
    // Called when a file in the browse view is activated.
    void onFileActivate(const QModelIndex&);
  signals:
    void setRootIndex(const QModelIndex&);
private:
    ConfigObject<ConfigValue>* m_pConfig;
    QFileSystemModel m_fileSystemModel;
    TrackCollection* m_pTrackCollection;
};

#endif /* BROWSEFEATURE_H */
