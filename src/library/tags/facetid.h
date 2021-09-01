#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>

#include "util/assert.h"

namespace mixxx {

namespace library {

namespace tags {

/// An identifier for referencing tag categories.
///
/// Facets are used for grouping/categorizing and providing context or meaning.
///
/// Serves as a symbolic, internal identifier that is not intended to be displayed
/// literally in the UI. The restrictive nameing constraints ensure that they are
/// not used for storing arbitrary text. Instead facet identifiers should be mapped
/// to translated display strings, e.g. the facet "genre" could be mapped to "Genre"
/// in English and the facet "venue" could be mapped to "Veranstaltungsort" in German.
///
/// Value constraints:
///   - charset/alphabet: +-./0123456789@[]_abcdefghijklmnopqrstuvwxyz
///   - no leading/trailing/inner whitespace
///
/// Rationale for the value constraints:
///   - Facet identifiers are intended to be created, shared, and parsed worldwide
///   - The Lingua franca of IT is English
///   - ASCII characters can be encoded by a single byte in UTF-8
///
/// References:
///   - https://en.wikipedia.org/wiki/Faceted_classification
class FacetId final {
  public:
    typedef QString value_t;

    /// The alphabet of facets
    ///
    /// All valid characters, ordered by their ASCII codes.
    static constexpr const char* kAlphabet = "+-./0123456789@[]_abcdefghijklmnopqrstuvwxyz";

    static bool isValidValue(
            const value_t& value);

    /// Convert the given string into lowercase and then
    /// removes all whitespace and non-ASCII characters.
    static value_t convertIntoValidValue(
            const value_t& value);

    /// Ensure that empty values are always null
    static value_t filterEmptyValue(
            value_t value) {
        // std::move() is required despite Return Value Optimization (RVO)
        // to avoid clazy warnings!
        return value.isEmpty() ? value_t{} : std::move(value);
    }

    /// Default constructor.
    FacetId() = default;

    /// Create a new instance.
    ///
    /// This constructor must not be used for static constants!
    explicit FacetId(
            value_t value)
            : m_value(std::move(value)) {
        DEBUG_ASSERT(isValid());
    }

    /// Type-tag for creating non-validated, static constants.
    ///
    /// The regular expressions required for validation are also
    /// static constant defined in this compilation unit. The
    /// initialization order between compilation units is undefined!
    enum struct StaticCtor {};

    /// Constructor for creating non-validated, static constants.
    FacetId(
            StaticCtor,
            value_t value)
            : m_value(std::move(value)) {
    }

    static FacetId staticConst(value_t value) {
        return FacetId(StaticCtor{}, std::move(value));
    }

    FacetId(const FacetId&) = default;
    FacetId(FacetId&&) = default;

    FacetId& operator=(const FacetId&) = default;
    FacetId& operator=(FacetId&&) = default;

    bool isValid() const {
        return isValidValue(m_value);
    }

    bool isEmpty() const {
        DEBUG_ASSERT(isValid());
        return m_value.isEmpty();
    }

    const value_t& value() const {
        DEBUG_ASSERT(isValid());
        return m_value;
    }
    operator const value_t&() const {
        return value();
    }

  private:
    value_t m_value;
};

inline bool operator==(
        const FacetId& lhs,
        const FacetId& rhs) {
    return lhs.value() == rhs.value();
}

inline bool operator!=(
        const FacetId& lhs,
        const FacetId& rhs) {
    return !(lhs == rhs);
}

inline bool operator<(
        const FacetId& lhs,
        const FacetId& rhs) {
    return lhs.value() < rhs.value();
}

inline bool operator>(
        const FacetId& lhs,
        const FacetId& rhs) {
    return !(lhs < rhs);
}

inline bool operator<=(
        const FacetId& lhs,
        const FacetId& rhs) {
    return !(lhs > rhs);
}

inline bool operator>=(
        const FacetId& lhs,
        const FacetId& rhs) {
    return !(lhs < rhs);
}

inline uint qHash(
        const FacetId& facetId,
        uint seed = 0) {
    return qHash(facetId.value(), seed);
}

/// Some predefined facets, mostly adopting the corresponding MusicBrainz
/// Picard naming conventions ("Internal Name") if available and suitable.
///
/// https://picard-docs.musicbrainz.org/en/variables/variables.html
/// https://picard-docs.musicbrainz.org/en/appendices/tag_mapping.html
///
/// TODO: These predefined facets are only intended as a starting point
/// and for inspiration. They could be modified as needed.
extern const FacetId kFacetComment; // multi-valued comment(s)
extern const FacetId kFacetDecade;  // e.g. "1980s"
extern const FacetId kFacetEthno;
extern const FacetId kFacetGenre;    // multi-valued genre(s)
extern const FacetId kFacetGrouping; // aka content group
extern const FacetId kFacetIsrc;     // ISRC
extern const FacetId kFacetLanguage; // ISO 639-3
extern const FacetId kFacetMood;     // multi-valued mood(s)
extern const FacetId kFacetVenue;
extern const FacetId kFacetVibe;
extern const FacetId kFacetAcoustidFingerprint;
extern const FacetId kFacetAcoustidId;
extern const FacetId kFacetMusicBrainzAlbumArtistId;
extern const FacetId kFacetMusicBrainzAlbumId;
extern const FacetId kFacetMusicBrainzArtistId;
extern const FacetId kFacetMusicBrainzRecordingId;
extern const FacetId kFacetMusicBrainzReleaseGroupId;
extern const FacetId kFacetMusicBrainzReleaseTrackId;
extern const FacetId kFacetMusicBrainzTrackId;
extern const FacetId kFacetMusicBrainzWorkId;

/// The score of this facet captures a user's subjective like (or dislike)
/// level.
///
/// The label is optional and used for discrimination. It may either refer
/// to an organization or contains the personal e-mail address of the
/// owner if known.
///
/// The normalized score is usually mapped to a star rating, typically
/// ranging from 0 to 5 stars.
extern const FacetId kFacetRating;

/// Predefined musical or audio feature scores as of Spotify/EchoNest.
///
/// A label is optional and could be used for identifying the source of
/// the assigned score. If the source it unspecified or unknown it should
/// be empty (= absent).
///
/// The combination of kFacetArousal and kFacetValence could
/// be used for classifying emotion (= mood) according to Thayer's
/// arousel-valence emotion plane.
extern const FacetId kFacetArousal; // for emotion classification
extern const FacetId kFacetValence; // for emotion classification
extern const FacetId kFacetAcousticness;
extern const FacetId kFacetDanceability;
extern const FacetId kFacetEnergy;
extern const FacetId kFacetInstrumentalness;
extern const FacetId kFacetLiveness;
extern const FacetId kFacetPopularity;
extern const FacetId kFacetSpeechiness;

extern const QVector<FacetId> kReservedFacetIds;

inline bool isReservedFacetId(
        const FacetId& facetId) {
    return kReservedFacetIds.contains(facetId);
}

} // namespace tags

} // namespace library

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::library::tags::FacetId)
