#pragma once

#include <QDir>
#include <QList>
#include <QString>

#include "library/relocatedtrack.h"
#include "preferences/usersettings.h"

class Library;
class LibraryFeature;
class Track;

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

    // Identifying name, e.g. for actions and menus
    virtual QString name() const = 0;

    // Single line description, e.g. for tool tips
    virtual QString description() const = 0;

    // Asynchronously establish the connection with the external
    // collection, e.g. by starting a separate worker thread or
    // by performing non-blocking network requests.
    virtual void establishConnection() = 0;

    // Synchronously (blocking!) stop the synchronization by
    // finishing all pending tasks and then disconnect from
    // the external collection. Disconnecting might be performed
    // asynchronously as long as no new synchronization tasks
    // are accepted and if aborting this operation prematurely
    // is safe, e.g. when shutting down Mixxx.
    virtual void finishPendingTasksAndDisconnect() = 0;

    enum class ConnectionState {
        Connecting,
        Connected,
        Disconnecting,
        Disconnected,
    };

    // Check if the connection to the external track collection
    // has been established, i.e. if the synchronization is active.
    virtual ConnectionState connectionState() const = 0;

    // Utility function for convenience
    /*non-virtual*/ bool isConnected() const {
        return connectionState() == ConnectionState::Connected;
    }

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

    // The default implementation first purges all missing track
    // locations and then updates all merged tracks.
    virtual /*async*/ void relocateTracks(
            const QList<RelocatedTrack>& relocatedTracks);

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
            Library* /*library*/,
            UserSettingsPointer /*userSettings*/) {
        return nullptr;
    }

  signals:
    void connectionStateChanged(ConnectionState state);

  protected:
    explicit ExternalTrackCollection(QObject* parent = nullptr)
            : QObject(parent) {
    }
};
