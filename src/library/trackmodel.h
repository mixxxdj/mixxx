#pragma once

#include <QItemDelegate>
#include <QList>
#include <QVector>
#include <QtSql>

#include "library/coverart.h"
#include "library/dao/settingsdao.h"
#include "track/track_decl.h"
#include "track/trackref.h"

/** Pure virtual (abstract) class that provides an interface for data models which
    display track lists. */
class TrackModel {
  public:
    static const int kHeaderWidthRole = Qt::UserRole + 0;
    static const int kHeaderNameRole = Qt::UserRole + 1;
    // This role is used for data export like in CSV files
    static constexpr int kDataExportRole = Qt::UserRole + 2;

    TrackModel(const QSqlDatabase& db,
            const char* settingsNamespace)
            : m_db(db),
              m_settingsNamespace(settingsNamespace),
              m_iDefaultSortColumn(-1),
              m_eDefaultSortOrder(Qt::AscendingOrder) {
    }
    virtual ~TrackModel() {}

    // These enums are the bits in a bitvector. Any individual column cannot
    // have a value other than 0, 1, 2, 4, or 8!
    enum Capabilities {
        TRACKMODELCAPS_NONE              = 0x00000,
        TRACKMODELCAPS_REORDER           = 0x00001,
        TRACKMODELCAPS_RECEIVEDROPS      = 0x00002,
        TRACKMODELCAPS_ADDTOPLAYLIST     = 0x00004,
        TRACKMODELCAPS_ADDTOCRATE        = 0x00008,
        TRACKMODELCAPS_ADDTOAUTODJ       = 0x00010,
        TRACKMODELCAPS_LOCKED            = 0x00020,
        TRACKMODELCAPS_EDITMETADATA      = 0x00040,
        TRACKMODELCAPS_LOADTODECK        = 0x00080,
        TRACKMODELCAPS_LOADTOSAMPLER     = 0x00100,
        TRACKMODELCAPS_LOADTOPREVIEWDECK = 0x00200,
        TRACKMODELCAPS_REMOVE            = 0x00400,
        TRACKMODELCAPS_RESETPLAYED       = 0x02000,
        TRACKMODELCAPS_HIDE              = 0x04000,
        TRACKMODELCAPS_UNHIDE            = 0x08000,
        TRACKMODELCAPS_PURGE             = 0x10000,
        TRACKMODELCAPS_REMOVE_PLAYLIST   = 0x20000,
        TRACKMODELCAPS_REMOVE_CRATE      = 0x40000,
    };
    typedef int CapabilitiesFlags; /** Enables us to do ORing */

    // Note that these enum values are used literally by controller scripts and must never be changed!
    // Both reordering or insertion of new enum variants is strictly forbidden!
    // New variants must always be inserted between the last valid and before the terminating variant IdMax!
    enum class SortColumnId : int {
        Invalid = -1,
        CurrentIndex = 0, // Column with the cursor on it
        Artist = 1,
        Title = 2,
        Album = 3,
        AlbumArtist = 4,
        Year = 5,
        Genre = 6,
        Composer = 7,
        Grouping = 8,
        TrackNumber = 9,
        FileType = 10,
        NativeLocation = 11,
        Comment = 12,
        Duration = 13,
        BitRate = 14,
        Bpm = 15,
        ReplayGain = 16,
        DateTimeAdded = 17,
        TimesPlayed = 18,
        Rating = 19,
        Key = 20,
        Preview = 21,
        CoverArt = 22,
        Position = 23,
        PlaylistId = 24,
        Location = 25,
        Filename = 26,
        FileModifiedTime = 27,
        FileCreationTime = 28,
        SampleRate = 29,
        Color = 30,

        // IdMax terminates the list of columns, it must be always after the last item
        IdMax,

        IdMin = Artist,
        NumOfIds = (IdMax - IdMin) + 1
    };

    // Deserialize and return the track at the given QModelIndex
    // or TrackRef in this result set.
    virtual TrackPointer getTrack(const QModelIndex& index) const = 0;
    virtual TrackPointer getTrackByRef(const TrackRef& trackRef) const = 0;

    // Gets the on-disk location of the track at the given location
    // with Qt separator "/".
    // Use QDir::toNativeSeparators() before displaying this to a user.
    virtual QString getTrackLocation(const QModelIndex& index) const = 0;

    // Gets the track ID of the track at the given QModelIndex
    virtual TrackId getTrackId(const QModelIndex& index) const = 0;

    virtual CoverInfo getCoverInfo(const QModelIndex& index) const = 0;

    // Gets the rows of the track in the current result set. Returns an
    // empty list if the track ID is not present in the result set.
    virtual const QVector<int> getTrackRows(TrackId trackId) const = 0;

    bool isTrackModel() { return true;}
    virtual void search(const QString& searchText, const QString& extraFilter=QString()) = 0;
    virtual const QString currentSearch() const = 0;
    virtual bool isColumnInternal(int column) = 0;
    // if no header state exists, we may hide some columns so that the user can
    // reactivate them
    virtual bool isColumnHiddenByDefault(int column) = 0;
    virtual const QList<int>& showableColumns() const { return m_emptyColumns; }
    virtual const QList<int>& searchColumns() const { return m_emptyColumns; }

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
    virtual int addTracks(const QModelIndex& index, const QList<QString>& locations) {
        Q_UNUSED(index);
        Q_UNUSED(locations);
        return 0;
    }
    virtual void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex) {
        Q_UNUSED(sourceIndex);
        Q_UNUSED(destIndex);
    }
    virtual bool isLocked() {
        return false;
    }
    virtual QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent) {
        Q_UNUSED(i);
        Q_UNUSED(pParent);
        return NULL;
    }
    virtual TrackModel::CapabilitiesFlags getCapabilities() const {
        return TRACKMODELCAPS_NONE;
    }
    /*non-virtual*/ bool hasCapabilities(TrackModel::CapabilitiesFlags caps) const {
        return (getCapabilities() & caps) == caps;
    }
    virtual QString getModelSetting(const QString& name) {
        SettingsDAO settings(m_db);
        QString key = m_settingsNamespace + "." + name;
        return settings.getValue(key);
    }

    virtual bool setModelSetting(const QString& name, const QVariant& value) {
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

    virtual bool isColumnSortable(int column) const {
        Q_UNUSED(column);
        return true;
    }

    virtual SortColumnId sortColumnIdFromColumnIndex(int index) const = 0;

    virtual int columnIndexFromSortColumnId(TrackModel::SortColumnId sortColumn) const = 0;

    virtual int fieldIndex(const QString& fieldName) const {
        Q_UNUSED(fieldName);
        return -1;
    }

    virtual void select() {
    }

  private:
    QSqlDatabase m_db;
    QString m_settingsNamespace;
    QList<int> m_emptyColumns;
    int m_iDefaultSortColumn;
    Qt::SortOrder m_eDefaultSortOrder;
};
