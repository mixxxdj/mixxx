#pragma once

#include <QDir>
#include <QList>
#include <QString>

#include "track/trackref.h"


class Track;

class LibraryFeature;

// This interface and base class enable to synchronize external
// track collections with Mixxx. It provides methods that will
// be invoked by Mixxx after tracks have been added, modified or
// deleted in the internal track collection. It also notifies
// external track collections if the metadata of a single track
// has been saved. A track in the internal track collection always
// refers to a single, local file.
//
// All functions must be implemented in a non-blocking fashion,
// i.e. asynchronously. They will be invoked AFTER the corresponding
// operation has been executed on the internal track collection.
//
// WARNING: External track collections MUST NOT modify the track
// files while Mixxx is running to avoid file corruption caused by
// concurrent write access!
class ExternalTrackCollection : public QObject {
Q_OBJECT

  public:
    virtual ~ExternalTrackCollection() = default;

    virtual QString name() const = 0;

    // Check if the connection to the extenal track collection
    // has been established, i.e. if the synchronization is active.
    virtual bool isActive() const = 0;

    // Synchronously (blocking) stop the synchronization by
    // finishing all pending requests.
    virtual void shutdown() {}

    // All tracks in the corresponding directory need to be
    // relocated recursively by updating their location.
    virtual /*async*/ void relocateDirectory(
            const QString& oldRootDir,
            const QString& newRootDir) = 0;

    // A (potentially large) number of tracks has recently been
    // modified by a batch update in the internal track collection.
    // The metadata of those tracks might need to be loaded in order
    // to send it to the external track collection.
    virtual /*async*/ void updateTracks(
            const QList<TrackRef>& updatedTracks) = 0;

    // The tracks referenced by their (local) file path have been
    // removed from the track collection and may also have disappeared
    // from the file system.
    virtual /*async*/ void purgeTracks(
            const QList<QString>& trackLocations) = 0;

    // All tracks in the corresponding directory have been removed
    // recursively from the root directory and the directory may
    // have disappeared from the file system.
    virtual /*async*/ void purgeAllTracks(
            const QDir& rootDir) = 0;

    // Duplications have been resolved by removing the duplicate track
    // and replacing any references with the corresponding replacement
    // track.
    // The default implementation first purges all duplicate tracks that
    // have been removed and then updates all the replaced tracks.
    struct DuplicateTrack {
        TrackRef removed;
        TrackRef replacedBy;
    };
    virtual /*async*/ void deduplicateTracks(
            const QList<DuplicateTrack>& duplicateTracks);

    // A new track has been added to the internal track collection or the
    // modified metadata of an existing track has just been saved.
    enum class ChangeHint {
        Added,
        Modified,
    };
    virtual /*async*/ void saveTrack(
            const Track& track,
            ChangeHint changeHint) = 0;

    // Create the corresponding library feature (if desired) that will
    // be hooked into the side pane in Mixxx.
    virtual LibraryFeature* newLibraryFeature(
            QObject* /*parent*/) {
        return nullptr;
    }

  protected:
    explicit ExternalTrackCollection(QObject* parent = nullptr)
            : QObject(parent) {
    }
};
