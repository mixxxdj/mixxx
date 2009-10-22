#ifndef TRACKMODEL_H
#define TRACKMODEL_H

#include <QList>
#include <QItemDelegate>

class TrackInfoObject;

/** Pure virtual (abstract) class that provides an interface for data models which
    display track lists. */
class TrackModel {

public:

    enum Capabilities
    {
        TRACKMODELCAPS_NONE           = 0x0000,
        TRACKMODELCAPS_REORDER        = 0x0001,
        TRACKMODELCAPS_RECEIVEDROPS   = 0x0002,
                                    //0x0004
    };

    typedef int CapabilitiesFlags; /** Enables us to do ORing */

    virtual TrackInfoObject* getTrack(const QModelIndex& index) const = 0;
    virtual QString getTrackLocation(const QModelIndex& index) const = 0;
    bool isTrackModel() { return true;}
    virtual void search(const QString& searchText) = 0;
    virtual const QString currentSearch() = 0;
    virtual const QList<int>& searchColumns() const { return m_emptyColumns; }
    virtual void removeTrack(const QModelIndex& index) = 0;
    virtual void addTrack(const QModelIndex& index, QString location) = 0;
    virtual void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex) = 0;
    virtual QItemDelegate* delegateForColumn(const int i) = 0;
    virtual ~TrackModel() {}
    virtual TrackModel::CapabilitiesFlags getCapabilities() const { return TRACKMODELCAPS_NONE; }

  private:
    QList<int> m_emptyColumns;
};

#endif
