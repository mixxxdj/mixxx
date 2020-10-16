#pragma once

/// Forward declarations for Track objects and their pointers

#include <QList>
#include <QMetaType>
#include <memory>

class Track;

typedef std::shared_ptr<Track> TrackPointer;
typedef std::weak_ptr<Track> TrackWeakPointer;
typedef QList<TrackPointer> TrackPointerList;

enum class ExportTrackMetadataResult {
    Succeeded,
    Failed,
    Skipped,
};

Q_DECLARE_METATYPE(TrackPointer);
