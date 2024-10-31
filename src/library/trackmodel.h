#pragma once

#include <QItemDelegate>
#include <QList>
#include <QUrl>
#include <QVector>
#include <QtSql>

#include "library/coverart.h"
#include "library/dao/settingsdao.h"
#include "track/track_decl.h"
#include "track/trackref.h"

/// Pure virtual (abstract) class that provides an interface for data models
/// which display track lists.
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
              m_eDefaultSortOrder(Qt::AscendingOrder),
              m_confirmHideRemoveTracks(true) {
    }
    virtual ~TrackModel() {}

    // These enums are the bits in a bitvector. Any individual column cannot
    // have a value other than 0, 1, 2, or 4!
    enum class Capability {
        None = 0u,
        Reorder = 1u << 0u,
        ReceiveDrops = 1u << 1u,
        AddToTrackSet = 1u << 2u,
        AddToAutoDJ = 1u << 3u,
        Locked = 1u << 4u,
        EditMetadata = 1u << 5u,
        LoadToDeck = 1u << 6u,
        LoadToSampler = 1u << 7u,
        LoadToPreviewDeck = 1u << 8u,
        Remove = 1u << 9u,
        ResetPlayed = 1u << 10u,
        Hide = 1u << 11u,
        Unhide = 1u << 12u,
        Purge = 1u << 13u,
        RemovePlaylist = 1u << 14u,
        RemoveCrate = 1u << 15u,
        RemoveFromDisk = 1u << 16u,
        Analyze = 1u << 17u,
        Properties = 1u << 18u,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

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
        LastPlayedAt = 31,
        PlaylistDateTimeAdded = 32,

        // IdMax terminates the list of columns, it must be always after the last item
        IdMax,

        IdMin = Artist,
        NumOfIds = (IdMax - IdMin) + 1
    };

    // Deserialize and return the track at the given QModelIndex
    // or TrackRef in this result set.
    virtual TrackPointer getTrack(const QModelIndex& index) const = 0;
    virtual TrackPointer getTrackByRef(const TrackRef& trackRef) const = 0;

    /// Get the URL of the track at the given QModelIndex.
    ///
    /// This function should be used in favor of getTrackId() to allow
    /// decoupling the TrackModel from the internal database. It should
    /// also be preferred over getTrackLocation() which implicitly
    /// assumes that tracks are always stored on the local file system.
    /// Using URLs for identifying tracks is more versatile.
    virtual QUrl getTrackUrl(const QModelIndex& index) const = 0;

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
    virtual int getTrackRowByPosition(int position) const {
        Q_UNUSED(position);
        return -1;
    }

    virtual const QList<int> getSelectedPositions(const QModelIndexList& indices) const {
        Q_UNUSED(indices);
        return {};
    }

    virtual void search(const QString& searchText, const QString& extraFilter=QString()) = 0;
    virtual const QString currentSearch() const = 0;
    virtual bool isColumnInternal(int column) = 0;
    // if no header state exists, we may hide some columns so that the user can
    // reactivate them
    virtual bool isColumnHiddenByDefault(int column) = 0;
    virtual const QList<int>& searchColumns() const { return m_emptyColumns; }

    virtual void removeTracks(const QModelIndexList& indices) {
        Q_UNUSED(indices);
    }
    virtual void cutTracks(const QModelIndexList& indices) {
        Q_UNUSED(indices);
    }
    virtual void copyTracks(const QModelIndexList& indices) const {
        Q_UNUSED(indices);
    }
    virtual QList<int> pasteTracks(const QModelIndex& index) {
        Q_UNUSED(index);
        return QList<int>();
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
    virtual int addTracksWithTrackIds(const QModelIndex& index,
            const QList<TrackId>& tracks,
            int* pOutInsertionPos) {
        Q_UNUSED(index);
        Q_UNUSED(tracks);
        Q_UNUSED(pOutInsertionPos);
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
    virtual Capabilities getCapabilities() const {
        return Capability::None;
    }
    /*non-virtual*/ bool hasCapabilities(Capabilities caps) const {
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

    /// @brief modelKey returns a unique identifier for the model
    /// @param noSearch don't include the current search in the key
    virtual QString modelKey(bool noSearch) const = 0;

    virtual bool getRequireConfirmationToHideRemoveTracks() {
        return m_confirmHideRemoveTracks;
    }
    virtual void setRequireConfirmationToHideRemoveTracks(bool require) {
        m_confirmHideRemoveTracks = require;
    }

    virtual bool updateTrackGenre(
            Track* pTrack,
            const QString& genre) const = 0;
#if defined(__EXTRA_METADATA__)
    virtual bool updateTrackMood(
            Track* pTrack,
            const QString& mood) const = 0;
#endif // __EXTRA_METADATA__

  private:
    QSqlDatabase m_db;
    QString m_settingsNamespace;
    QList<int> m_emptyColumns;
    int m_iDefaultSortColumn;
    Qt::SortOrder m_eDefaultSortOrder;
    bool m_confirmHideRemoveTracks;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(TrackModel::Capabilities)
