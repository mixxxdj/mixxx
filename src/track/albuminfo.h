#pragma once

#include <QString>
#include <QUuid>

#include "track/replaygain.h"

#include "util/macros.h"


namespace mixxx {

class AlbumInfo final {
    // Properties in alphabetical order
    MIXXX_DECL_PROPERTY(QString, artist, Artist)
#if defined(__EXTRA_METADATA__)
    MIXXX_DECL_PROPERTY(QString, copyright, Copyright)
    MIXXX_DECL_PROPERTY(QString, license, License)
    MIXXX_DECL_PROPERTY(QUuid, musicBrainzArtistId, MusicBrainzArtistId)
    MIXXX_DECL_PROPERTY(QUuid, musicBrainzReleaseGroupId, MusicBrainzReleaseGroupId)
    MIXXX_DECL_PROPERTY(QUuid, musicBrainzReleaseId, MusicBrainzReleaseId)
    MIXXX_DECL_PROPERTY(QString, recordLabel, RecordLabel)
    MIXXX_DECL_PROPERTY(ReplayGain, replayGain, ReplayGain)
#endif // __EXTRA_METADATA__
    MIXXX_DECL_PROPERTY(QString, title, Title)

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
