// libraryfeature.h
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#ifndef LIBRARYFEATURE_H
#define LIBRARYFEATURE_H

#include <QVariant>
#include <QObject>
#include <QModelIndex>
#include <QIcon>

class TrackModel;

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
    virtual void showTrackTableView(TrackModel* model);
};

#endif /* LIBRARYFEATURE_H */
