#pragma once

#include <QDir>
#include <QList>
#include <QString>

#include "track/trackref.h"


class Track;

class LibraryFeature;

class ExternalTrackCollection : public QObject {
Q_OBJECT

  public:
    virtual ~ExternalTrackCollection() = default;

    virtual QString name() const = 0;

    virtual bool isActive() const = 0;

    virtual void shutdown() {}

    virtual /*async*/ void relocateDirectory(
            const QString& oldDir,
            const QString& newDir) = 0;

    virtual /*async*/ void updateTracks(
            const QList<TrackRef>& updatedTracks) = 0;

    virtual /*async*/ void purgeTracks(
            const QList<QString>& trackLocations) = 0;
    virtual /*async*/ void purgeAllTracks(
            const QDir& rootDir) = 0;

    // The default implementation first purges all tracks that are
    // supposed to be removed and then updates all corresponding
    // replaced tracks.
    struct DuplicateTrack {
        TrackRef removed;
        TrackRef replacedBy;
    };
    virtual /*async*/ void deduplicateTracks(
            const QList<DuplicateTrack>& duplicateTracks);

    enum class ChangeHint {
        Added,
        Modified,
    };
    virtual /*async*/ void saveTrack(
            const Track& track,
            ChangeHint changeHint) = 0;

    virtual LibraryFeature* newLibraryFeature(
            QObject* parent) = 0;

  protected:
    explicit ExternalTrackCollection(QObject* parent = nullptr)
            : QObject(parent) {
    }
};
