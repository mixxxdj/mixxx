#pragma once

#include <QString>
#include <QUuid>

#include "track/replaygain.h"

#include "util/macros.h"


namespace mixxx {

class AlbumInfo final {
    // Properties in alphabetical order
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    artist,                    Artist)
#if defined(__EXTRA_METADATA__)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    copyright,                 Copyright)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    license,                   License)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzArtistId,       MusicBrainzArtistId)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzReleaseGroupId, MusicBrainzReleaseGroupId)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzReleaseId,      MusicBrainzReleaseId)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    recordLabel,               RecordLabel)
    PROPERTY_SET_BYVAL_GET_BYREF(ReplayGain, replayGain,                ReplayGain)
#endif // __EXTRA_METADATA__
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    title,                     Title)

public:
    AlbumInfo() = default;
    AlbumInfo(AlbumInfo&&) = default;
    AlbumInfo(const AlbumInfo&) = default;
    /*non-virtual*/ ~AlbumInfo() = default;

    AlbumInfo& operator=(AlbumInfo&&) = default;
    AlbumInfo& operator=(const AlbumInfo&) = default;

    // Adjusts floating-point properties to match their string representation
    // in file tags to account for rounding errors.
    void normalizeBeforeExport() {
#if defined(__EXTRA_METADATA__)
        refReplayGain().normalizeBeforeExport();
#endif // __EXTRA_METADATA__
    }
};

bool operator==(const AlbumInfo& lhs, const AlbumInfo& rhs);

inline
bool operator!=(const AlbumInfo& lhs, const AlbumInfo& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const AlbumInfo& arg);

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::AlbumInfo)
