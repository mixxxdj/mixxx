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

class TrackModel;
class TrackInfoObject;

class LibraryFeature : public QObject {
  Q_OBJECT
  public:
    LibraryFeature(QObject* parent = NULL);
    
    virtual QVariant title() = 0;
    virtual QIcon getIcon() = 0;
    virtual int numChildren() = 0;
    virtual QVariant child(int n) = 0;

public slots:
    virtual void activate() = 0;
    virtual void activateChild(int n) = 0;
    virtual void onRightClick(QModelIndex index) = 0;
    virtual void onClick(QModelIndex index) = 0;
signals:
    virtual void featureUpdated();
    virtual void showTrackModel(QAbstractItemModel* model);
    virtual void showView(QString view);
};

#endif /* LIBRARYFEATURE_H */
