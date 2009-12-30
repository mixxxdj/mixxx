// libraryfeature.h
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#ifndef LIBRARYFEATURE_H
#define LIBRARYFEATURE_H

#include <QtDebug>
#include <QIcon>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QAbstractItemModel>
#include <QUrl>

class TrackModel;
class TrackInfoObject;
class WLibrarySidebar;
class WLibrary;

class LibraryFeature : public QObject {
  Q_OBJECT
  public:
    LibraryFeature(QObject* parent = NULL);

    virtual QVariant title() = 0;
    virtual QIcon getIcon() = 0;

    virtual bool dropAccept(QUrl url) = 0;
    virtual bool dropAcceptChild(const QModelIndex& index, QUrl url) = 0;
    virtual bool dragMoveAccept(QUrl url) = 0;
    virtual bool dragMoveAcceptChild(const QModelIndex& index, QUrl url) = 0;

    // Reimplement this to register custom views with the library widget.
    virtual void bindWidget(WLibrarySidebar* sidebarWidget,
                            WLibrary* libraryWidget) {
    }
    virtual QAbstractItemModel* getChildModel() = 0;

  public slots:
    virtual void activate() = 0;
    virtual void activateChild(const QModelIndex& index) = 0;
    virtual void onRightClick(const QPoint& globalPos) = 0;
    virtual void onRightClickChild(const QPoint& globalPos, QModelIndex index) = 0;
  signals:
    void featureUpdated();
    void showTrackModel(QAbstractItemModel* model);
    void switchToView(const QString& view);
    void loadTrack(TrackInfoObject* pTrack);
    void loadTrackToPlayer(TrackInfoObject* pTrack, int player);
    void restoreSearch(const QString&);
};

#endif /* LIBRARYFEATURE_H */
