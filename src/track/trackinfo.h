#pragma once

#include <QString>
#include <QUuid>

#include "sources/audiosource.h"

#include "track/bpm.h"
#include "track/replaygain.h"

#include "util/duration.h"
#include "util/macros.h"


namespace mixxx {

class TrackInfo final {
    // Track properties (in alphabetical order)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    artist,               Artist)
    PROPERTY_SET_BYVAL_GET_BYREF(Bpm,        bpm,                  Bpm)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    comment,              Comment)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    composer,             Composer)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    conductor,            Conductor)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    discNumber,           DiscNumber)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    discTotal,            DiscTotal)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    encoder,              Encoder)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    encoderSettings,      EncoderSettings)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    genre,                Genre)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    grouping,             Grouping)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    key,                  Key)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    isrc,                 ISRC)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    language,             Language)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    lyricist,             Lyricist)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    mood,                 Mood)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    movement,             Movement)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzArtistId,  MusicBrainzArtistId)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzRecordingId, MusicBrainzRecordingId)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzReleaseId, MusicBrainzReleaseId)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzWorkId,    MusicBrainzWorkId)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    remixer,              Remixer)
    PROPERTY_SET_BYVAL_GET_BYREF(ReplayGain, replayGain,           ReplayGain)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    subtitle,             Subtitle)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    title,                Title)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    trackNumber,          TrackNumber)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    trackTotal,           TrackTotal)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    work,                 Work)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    year,                 Year) // = release date

public:
    TrackInfo() = default;
    TrackInfo(TrackInfo&&) = default;
    TrackInfo(const TrackInfo&) = default;
    /*non-virtual*/ ~TrackInfo() = default;

    TrackInfo& operator=(TrackInfo&&) = default;
    TrackInfo& operator=(const TrackInfo&) = default;

    // TODO(XXX): Remove after all new fields have been added to the library
    void resetUnsupportedValues();

    // Adjusts floating-point values to match their string representation
    // in file tags to account for rounding errors.
    void normalizeBeforeExport() {
        refBpm().normalizeBeforeExport();
        refReplayGain().normalizeBeforeExport();
    }
};

bool operator==(const TrackInfo& lhs, const TrackInfo& rhs);

inline
bool operator!=(const TrackInfo& lhs, const TrackInfo& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const TrackInfo& arg);

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::TrackInfo)
