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

enum class HotcueSortMode {
    KeepOffsets,
    RemoveOffsets,
};

// key for control to open/close the decks' track menus
const QString kShowTrackMenuKey = QStringLiteral("show_track_menu");

Q_DECLARE_METATYPE(TrackPointer);
