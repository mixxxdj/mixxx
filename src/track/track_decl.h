#pragma once

/// Forward declarations for Track objects and their pointers

#include <QList>
#include <QMetaType>
#include <memory>

#include "preferences/usersettings.h"

class Track;

typedef std::shared_ptr<Track> TrackPointer;
typedef std::weak_ptr<Track> TrackWeakPointer;
typedef QList<TrackPointer> TrackPointerList;

struct SyncTrackMetadataParams {
    // readFromUserSettings() depends on the ordering of the members!
    bool resetMissingTagMetadataOnImport = false;
    bool syncSeratoMetadata = false;

    static SyncTrackMetadataParams readFromUserSettings(
            const UserSettings& userSettings);
};

enum class ExportTrackMetadataResult {
    Succeeded,
    Failed,
    Skipped,
};

Q_DECLARE_METATYPE(TrackPointer);
