#pragma once

#include <QString>
#include <QUuid>

#include "sources/audiosource.h"
#include "track/bpm.h"
#include "track/replaygain.h"
#include "track/serato/tags.h"
#include "util/duration.h"
#include "util/macros.h"

namespace mixxx {

class TrackInfo final {
    // Properties in alphabetical order
    MIXXX_DECL_PROPERTY(QString, artist, Artist)
    MIXXX_DECL_PROPERTY(Bpm, bpm, Bpm)
    MIXXX_DECL_PROPERTY(QString, comment, Comment)
    MIXXX_DECL_PROPERTY(QString, composer, Composer)
#if defined(__EXTRA_METADATA__)
    MIXXX_DECL_PROPERTY(QString, conductor, Conductor)
    MIXXX_DECL_PROPERTY(QString, discNumber, DiscNumber)
    MIXXX_DECL_PROPERTY(QString, discTotal, DiscTotal)
    MIXXX_DECL_PROPERTY(QString, encoder, Encoder)
    MIXXX_DECL_PROPERTY(QString, encoderSettings, EncoderSettings)
#endif // __EXTRA_METADATA__
    MIXXX_DECL_PROPERTY(QString, genre, Genre)
    MIXXX_DECL_PROPERTY(QString, grouping, Grouping)
#if defined(__EXTRA_METADATA__)
    MIXXX_DECL_PROPERTY(QString, isrc, ISRC)
#endif // __EXTRA_METADATA__
    MIXXX_DECL_PROPERTY(QString, key, Key)
#if defined(__EXTRA_METADATA__)
    MIXXX_DECL_PROPERTY(QString, language, Language)
    MIXXX_DECL_PROPERTY(QString, lyricist, Lyricist)
    MIXXX_DECL_PROPERTY(QString, mood, Mood)
    MIXXX_DECL_PROPERTY(QString, movement, Movement)
    MIXXX_DECL_PROPERTY(QUuid, musicBrainzArtistId, MusicBrainzArtistId)
    MIXXX_DECL_PROPERTY(QUuid, musicBrainzRecordingId, MusicBrainzRecordingId)
    MIXXX_DECL_PROPERTY(QUuid, musicBrainzReleaseId, MusicBrainzReleaseId)
    MIXXX_DECL_PROPERTY(QUuid, musicBrainzWorkId, MusicBrainzWorkId)
    MIXXX_DECL_PROPERTY(QString, remixer, Remixer)
#endif // __EXTRA_METADATA__
    MIXXX_DECL_PROPERTY(ReplayGain, replayGain, ReplayGain)
    MIXXX_DECL_PROPERTY(SeratoTags, seratoTags, SeratoTags)
#if defined(__EXTRA_METADATA__)
    MIXXX_DECL_PROPERTY(QString, subtitle, Subtitle)
#endif // __EXTRA_METADATA__
    MIXXX_DECL_PROPERTY(QString, title, Title)
    MIXXX_DECL_PROPERTY(QString, trackNumber, TrackNumber)
    MIXXX_DECL_PROPERTY(QString, trackTotal, TrackTotal)
#if defined(__EXTRA_METADATA__)
    MIXXX_DECL_PROPERTY(QString, work, Work)
#endif // __EXTRA_METADATA__
    MIXXX_DECL_PROPERTY(QString, year, Year) // = release date

  public:
    TrackInfo() = default;
    TrackInfo(TrackInfo&&) = default;
    TrackInfo(const TrackInfo&) = default;
    /*non-virtual*/ ~TrackInfo() = default;

    TrackInfo& operator=(TrackInfo&&) = default;
    TrackInfo& operator=(const TrackInfo&) = default;

    // Returns true if modified
    bool parseArtistTitleFromFileName(
            QString fileName,
            bool splitArtistTitle);

    // Adjusts floating-point properties to match their string representation
    // in file tags to account for rounding errors.
    void normalizeBeforeExport() {
        m_bpm.normalizeBeforeExport();
        m_replayGain.normalizeBeforeExport();
    }

    bool compareEq(
            const TrackInfo& trackInfo,
            Bpm::Comparison cmpBpm = Bpm::Comparison::Default) const;
};

inline
bool operator==(const TrackInfo& lhs, const TrackInfo& rhs) {
    return lhs.compareEq(rhs);
}

inline
bool operator!=(const TrackInfo& lhs, const TrackInfo& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const TrackInfo& arg);

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::TrackInfo)
