#pragma once

#include "library/coverart.h"
#include "proto/keys.pb.h"
#include "track/cue.h"
#include "track/keys.h"
#include "track/keyutils.h"
#include "track/playcounter.h"
#include "track/trackid.h"
#include "track/trackmetadata.h"
#include "util/color/rgbcolor.h"


namespace mixxx {

/// Effect of updating a property with a new value.
enum class [[nodiscard]] UpdateResult{
        /// The value has been updated and changed.
        Updated,

        /// The value didn't change and has not been updated.
        Unchanged,

        /// The provided value is invalid or insonsistent with
        /// any existing value(s) and has been rejected, i.e.
        /// the current value didn't change either.
        Rejected,
};

// Properties of tracks that are stored in the database.
class TrackRecord final {
    // Properties that parsed from and (optionally) written back to their
    // source, i.e. the corresponding file
    MIXXX_DECL_PROPERTY(TrackMetadata, metadata, Metadata)

    // The unique ID of track. This value is only set once after the track
    // has been inserted or is loaded from the library DB.
    MIXXX_DECL_PROPERTY(TrackId, id, Id)

    // Both import and export of metadata can be tracked by a single time
    // stamp, the direction doesn't matter. The value should be set to the
    // modification time stamp provided by the metadata source. This would
    // enable us to update the metadata of all tracks in the database after
    // the external metadata has been modified, i.e. if the corresponding
    // files have been modified.
    MIXXX_DECL_PROPERTY(QDateTime, sourceSynchronizedAt, SourceSynchronizedAt)

    MIXXX_DECL_PROPERTY(CoverInfoRelative, coverInfo, CoverInfo)

    MIXXX_DECL_PROPERTY(QDateTime, dateAdded, DateAdded)
    MIXXX_DECL_PROPERTY(QString, fileType, FileType)
    MIXXX_DECL_PROPERTY(QString, url, Url)
    MIXXX_DECL_PROPERTY(PlayCounter, playCounter, PlayCounter)
    MIXXX_DECL_PROPERTY(RgbColor::optional_t, color, Color)
    MIXXX_DECL_PROPERTY(mixxx::audio::FramePos, mainCuePosition, MainCuePosition)
    MIXXX_DECL_PROPERTY(int, rating, Rating)
    MIXXX_DECL_PROPERTY(bool, bpmLocked, BpmLocked)

  public:
    // Data migration: Reload track total from file tags if not initialized
    // yet. The added column "tracktotal" has been initialized with the
    // default value "//".
    // See also: Schema revision 26 in schema.xml
    // Public only for testing purposes!
    static const QString kTrackTotalPlaceholder;

    explicit TrackRecord(TrackId id = TrackId());
    TrackRecord(TrackRecord&&) = default;
    TrackRecord(const TrackRecord&) = default;
    /*non-virtual*/ ~TrackRecord() = default;

    TrackRecord& operator=(TrackRecord&&) = default;
    TrackRecord& operator=(const TrackRecord&) = default;

    static constexpr int kMinRating = 0;
    static constexpr int kMaxRating = 5;
    static constexpr int kNoRating = kMinRating;

    static bool isValidRating(int rating) {
        return rating >= kMinRating && rating <= kMaxRating;
    }
    bool hasRating() const {
        return getRating() != kNoRating;
    }

    void setKeys(const Keys& keys);
    void resetKeys() {
        setKeys(Keys());
    }
    const Keys& getKeys() const {
        return m_keys;
    }

    track::io::key::ChromaticKey getGlobalKey() const {
        if (getKeys().isValid()) {
            return getKeys().getGlobalKey();
        } else {
            return track::io::key::INVALID;
        }
    }
    bool updateGlobalKey(
            track::io::key::ChromaticKey key,
            track::io::key::Source keySource);

    QString getGlobalKeyText() const {
        return KeyUtils::getGlobalKeyText(getKeys());
    }
    UpdateResult updateGlobalKeyText(
            const QString& keyText,
            track::io::key::Source keySource);

    bool isSourceSynchronized() const;
    bool replaceMetadataFromSource(
            TrackMetadata&& importedMetadata,
            const QDateTime& sourceSynchronizedAt);

    // Merge the current metadata with new and additional properties
    // imported from the file. Since these properties are not (yet)
    // stored in the library or have been added later all existing
    // data must be preserved and never be overwritten! This allows
    // a gradual migration by selectively reimporting the required
    // data when needed.
    //
    // Returns true if any property has been modified or false otherwise.
    bool mergeExtraMetadataFromSource(
            const TrackMetadata& importedMetadata);

    /// Update the stream info after opening the audio stream during
    /// a session.
    /// Returns true if the corresponding metadata properties have been
    /// updated and false otherwise.
    bool updateStreamInfoFromSource(
            mixxx::audio::StreamInfo streamInfoFromSource);
    /// Check if the stream info is supposed to be reliable and accurate.
    /// TODO: Also flag the stream info as "accurate" in the database and
    /// invoke updateStreamInfoFromSource() accordingly when loading tracks
    /// from the database.
    bool hasStreamInfoFromSource() const {
        return static_cast<bool>(m_streamInfoFromSource);
    }
    const std::optional<audio::StreamInfo>& getStreamInfoFromSource() const {
        return m_streamInfoFromSource;
    }

  private:
    // TODO: Remove this dependency
    friend class ::Track;

    bool updateSourceSynchronizedAt(
            const QDateTime& sourceSynchronizedAt);

    Keys m_keys;

    // TODO: Use TrackMetadata as single source of truth and do not
    // store this information redundantly.
    //
    // PROPOSAL (as implemented by https://gitlab.com/uklotzde/aoide-rs):
    // This redesign requires to track the status of some or all track
    // metadata (which includes the stream info properties) by a set of
    // bitflags:
    //  - UNRELIABLE = 0 (default)
    //    Parsed from file tags which are considered inaccurate and
    //    are often imprecise
    //  - RELIABLE =   1 << 0
    //    Reported by a decoder when opening an audio/video stream for
    //    reading. Nevertheless different decoders may report slightly
    //    differing values.
    //  - LOCKED =     1 << 1
    //    Locked metadata will not be updated automatically, neither when
    //    parsing file tags nor when decoding an audio/video stream.
    //    While locked the stale flag is never set.
    //  - STALE =      1 << 2
    //    Stale metadata should be re-imported depending on the other flags.
    std::optional<audio::StreamInfo> m_streamInfoFromSource;

    bool m_headerParsed; // deprecated, replaced by sourceSynchronizedAt

    /// Equality comparison
    ///
    /// Exception: The member m_streamInfoFromSource must not be considered
    /// for equality comparisons! It is only needed for verifying consistency
    /// during updates and as a flags when a track is loaded.
    friend bool operator==(const TrackRecord& lhs, const TrackRecord& rhs);
};

inline bool operator!=(const TrackRecord& lhs, const TrackRecord& rhs) {
    return !(lhs == rhs);
}

} // namespace mixxx
