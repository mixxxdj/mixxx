#ifndef TRACKMODEL_H
#define TRACKMODEL_H

#include <QItemDelegate>

class TrackInfoObject;

/** Pure virtual (abstract) class that provides an interface for data models which
    display track lists. */
class TrackModel {
public:
    virtual TrackInfoObject* getTrack(const QModelIndex& index) const = 0;
    virtual QString getTrackLocation(const QModelIndex& index) const = 0;
    bool isTrackModel() { return true;};
    virtual void search(const QString& searchText) = 0;
    virtual void removeTrack(const QModelIndex& index) = 0;
    virtual void addTrack(const QModelIndex& index, QString location) = 0;
    virtual QItemDelegate* delegateForColumn(const int i) = 0;
    virtual ~TrackModel() {};
};

#endif
