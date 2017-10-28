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
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    artist,        Artist)
    PROPERTY_SET_BYVAL_GET_BYREF(Bpm,        bpm,           Bpm)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    comment,       Comment)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    composer,      Composer)
    PROPERTY_SET_BYVAL_GET_BYREF(Duration,   duration,      Duration)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    genre,         Genre)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    grouping,      Grouping)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    key,           Key)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzId, MusicBrainzId)
    PROPERTY_SET_BYVAL_GET_BYREF(ReplayGain, replayGain,    ReplayGain)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    title,         Title)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    trackNumber,   TrackNumber)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    trackTotal,    TrackTotal)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    year,          Year)

    // Audio properties (in alphabetical order)
    PROPERTY_SET_BYVAL_GET_BYREF(AudioSource::Bitrate,      bitrate,    Bitrate)
    PROPERTY_SET_BYVAL_GET_BYREF(AudioSignal::ChannelCount, channels,   Channels)
    PROPERTY_SET_BYVAL_GET_BYREF(AudioSignal::SampleRate,   sampleRate, SampleRate)

public:
    TrackInfo() = default;
    TrackInfo(TrackInfo&&) = default;
    TrackInfo(const TrackInfo&) = default;
    /*non-virtual*/ ~TrackInfo() = default;

    TrackInfo& operator=(TrackInfo&&) = default;
    TrackInfo& operator=(const TrackInfo&) = default;
};

bool operator==(const TrackInfo& lhs, const TrackInfo& rhs);

inline
bool operator!=(const TrackInfo& lhs, const TrackInfo& rhs) {
    return !(lhs == rhs);
}

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::TrackInfo)
