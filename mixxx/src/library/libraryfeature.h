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

#include "trackinfoobject.h"

class TrackModel;
class WLibrarySidebar;
class WLibrary;
class MixxxKeyboard;

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
                            WLibrary* libraryWidget,
                            MixxxKeyboard* keyboard) {
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
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, int player);
    void restoreSearch(const QString&);
};

#endif /* LIBRARYFEATURE_H */
