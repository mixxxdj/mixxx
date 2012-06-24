#ifndef TRACKMODEL_H
#define TRACKMODEL_H

#include <QList>
#include <QLinkedList>
#include <QItemDelegate>
#include <QtSql>

#include "trackinfoobject.h"
#include "library/dao/settingsdao.h"

/** Pure virtual (abstract) class that provides an interface for data models which
    display track lists. */
class TrackModel {
  public:
    TrackModel(QSqlDatabase db,
               QString settingsNamespace)
            : m_db(db),
              m_settingsNamespace(settingsNamespace),
              m_iDefaultSortColumn(-1),
              m_eDefaultSortOrder(Qt::AscendingOrder) {
    }
    virtual ~TrackModel() {}

    enum Capabilities {
        TRACKMODELCAPS_NONE           = 0x0000,
        TRACKMODELCAPS_REORDER        = 0x0001,
        TRACKMODELCAPS_RECEIVEDROPS   = 0x0002,
        TRACKMODELCAPS_ADDTOPLAYLIST  = 0x0004,
        TRACKMODELCAPS_ADDTOCRATE     = 0x0008,
        TRACKMODELCAPS_ADDTOAUTODJ    = 0x0010,
        TRACKMODELCAPS_LOCKED         = 0x0020,
        TRACKMODELCAPS_RELOADMETADATA = 0x0040,
        TRACKMODELCAPS_LOADTODECK     = 0x0080,
        TRACKMODELCAPS_LOADTOSAMPLER  = 0x0100,
        TRACKMODELCAPS_REMOVE         = 0x0200,
        TRACKMODELCAPS_RELOCATE       = 0x0400,
        TRACKMODELCAPS_BPMLOCK        = 0x0800,
        TRACKMODELCAPS_CLEAR_BEATS    = 0x1000,
        TRACKMODELCAPS_RESETPLAYED    = 0x2000,
        TRACKMODELCAPS_HIDE           = 0x3000,
        TRACKMODELCAPS_UNHIDE         = 0x4000,
        TRACKMODELCAPS_PURGE          = 0x8000,
    };

    typedef int CapabilitiesFlags; /** Enables us to do ORing */

    // Deserialize and return the track at the given QModelIndex in this result
    // set.
    virtual TrackPointer getTrack(const QModelIndex& index) const = 0;

    // Gets the on-disk location of the track at the given location.
    virtual QString getTrackLocation(const QModelIndex& index) const = 0;

    // Gets the track ID of the track at the given QModelIndex
    virtual int getTrackId(const QModelIndex& index) const = 0;

    // Gets the row of the track in the current result set. Returns -1 if the
    // track ID is not present in the result set.
    virtual const QLinkedList<int> getTrackRows(int trackId) const = 0;

    bool isTrackModel() { return true;}
    virtual void search(const QString& searchText) = 0;
    virtual const QString currentSearch() const = 0;
    virtual bool isColumnInternal(int column) = 0;
    // if no header state exists, we may hide some columns so that the user can
    // reactivate them
    virtual bool isColumnHiddenByDefault(int column) = 0;
    virtual const QList<int>& showableColumns() const { return m_emptyColumns; }
    virtual const QList<int>& searchColumns() const { return m_emptyColumns; }
    virtual void removeTrack(const QModelIndex& index) {
        Q_UNUSED(index);
    }
    virtual void removeTracks(const QModelIndexList& indices) {
        Q_UNUSED(indices);
    }
    virtual void hideTracks(const QModelIndexList& indices) {
        Q_UNUSED(indices);
    }
    virtual void unhideTracks(const QModelIndexList& indices) {
        Q_UNUSED(indices);
    }
    virtual void purgeTracks(const QModelIndexList& indices) {
        Q_UNUSED(indices);
    }
    virtual bool addTrack(const QModelIndex& index, QString location) {
        Q_UNUSED(index);
        Q_UNUSED(location);
        return false;
    }
    virtual int addTracks(const QModelIndex& index, QList<QString> locations) {
        Q_UNUSED(index);
        Q_UNUSED(locations);
        return 0;
    }
    virtual void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex) {
        Q_UNUSED(sourceIndex);
        Q_UNUSED(destIndex);
    }
    virtual QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent) {
        Q_UNUSED(i);
        Q_UNUSED(pParent);
        return NULL;
    }
    virtual TrackModel::CapabilitiesFlags getCapabilities() const {
        return TRACKMODELCAPS_NONE;
    }
    virtual QString getModelSetting(QString name) {
        SettingsDAO settings(m_db);
        QString key = m_settingsNamespace + "." + name;
        return settings.getValue(key);
    }

    virtual bool setModelSetting(QString name, QVariant value) {
        SettingsDAO settings(m_db);
        QString key = m_settingsNamespace + "." + name;
        return settings.setValue(key, value);
    }

    virtual int defaultSortColumn() const {
        return m_iDefaultSortColumn;
    }

    virtual Qt::SortOrder defaultSortOrder() const {
        return m_eDefaultSortOrder;
    }

    virtual void setDefaultSort(int sortColumn, Qt::SortOrder sortOrder) {
        m_iDefaultSortColumn = sortColumn;
        m_eDefaultSortOrder = sortOrder;
    }

  private:
    QSqlDatabase m_db;
    QString m_settingsNamespace;
    QList<int> m_emptyColumns;
    int m_iDefaultSortColumn;
    Qt::SortOrder m_eDefaultSortOrder;
};

#endif
