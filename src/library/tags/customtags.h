#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QMap>

#include "library/tags/facetid.h"
#include "library/tags/tag.h"

namespace mixxx {

namespace library {

namespace tags {

typedef QMap<Label, Score> TagMap;
typedef QMap<FacetId, TagMap> FacetTagMap;

/// Custom tags
///
/// Each custom tag is represented by a 3-tuple (triple) of a *facet (`FacetId`),
/// a *label* (`Label`), and a *score* (`Score`):
///
///   - The *facet* is represented by a *facet identifier* string
///   - The *label* is non-empty, free text without leading/trailing whitespace
///   - The *score* is a floating-point value in the interval [0.0, 1.0]
///
/// Both *facet* and *label* are optional (= nullable), but at least one of them
/// must be present. The *score* is mandatory and is supposed to be interpreted
/// as a *weight*, *degree of cohesion*, or *normalized level* with a default value
/// of 1.0.
///
/// The following combinations are allowed, missing values are indicated by `-`:
///
///   - `(-, label, score)`, e.g. `label` = "Top 40", `score` = 1.0 for tagging a track with
///     the label "Top 40" and full (= no particular, i.e. binary) score. Instead of using
///     a score of 0.0 you would simply remove the entire tag then.
///   - `(facet, -, score)`, e.g. `facet` = "energy", `score` = 0.8 for tagging a track with
///     a fairly high energy level.
///   - `(facet, label, score)`, e.g. `facet` = "genre", `label` = "Hip Hop/Rap", `score` = 0.3
///     for tagging a track that resembles the genre "Hip Hop/Rap" to a fairly low amount and
///     might be a candidate during a genre change. The "genre" tag with the highest score will
///     represent the main genre and the score defines an ordering between multiple tags of the
///     same facet.
///
/// The tuple (`facet`, `label`) defines a uniqueness constraint among all custom tags, i.e. within
/// an instance of `CustomTags` which is basically two nested maps. The 1st key is `facet` (nullable)
/// and the 2nd key is `label` (nullable).
class CustomTags final {
    MIXXX_DECL_PROPERTY(FacetTagMap, facetedTags, FacetedTags)

  public:
    CustomTags() = default;
    CustomTags(CustomTags&&) = default;
    CustomTags(const CustomTags&) = default;
    CustomTags& operator=(CustomTags&&) = default;
    CustomTags& operator=(const CustomTags&) = default;

    /// Purge all empty and unused map entries
    void compact();

    /// Check for consistency
    bool validate() const;

    bool isEmpty() const;

    bool containsFacet(
            const FacetId& facetId) const {
        return getFacetedTags().contains(facetId);
    }

    /// Add an (empty) entry for the given facet if it does not exist yet.
    ///
    /// Existing entries are not affected.
    void addOrIgnoreFacet(
            const FacetId& facetId) {
        refFacetedTags()[facetId];
        DEBUG_ASSERT(containsFacet(facetId));
    }

    bool containsTag(
            const Label& label,
            const FacetId& facetId = FacetId()) const;
    int countTags(
            const Label& label,
            const FacetId& facetId = FacetId()) const;

    int countTags() const {
        return countFacetedTags(FacetId());
    }
    TagMap& refTags() {
        return refFacetedTags()[FacetId()];
    }

    int countFacetedTags(
            const FacetId& facetId) const;
    TagMap& refFacetedTags(
            const FacetId& facetId) {
        return refFacetedTags()[facetId];
    }

    bool addOrReplaceTag(
            const Tag& tag,
            const FacetId& facetId = FacetId());
    bool removeTag(
            const Label& label,
            const FacetId& facetId = FacetId());

    bool addOrReplaceAllTags(
            const CustomTags& tags);

    /// Get all plain tags as a list (in no particular order)
    TagVector getTags() const;

    /// Replace all existing tags of this facet with a single
    /// faceted tag or insert a new faceted tag.
    FacetTagMap::iterator replaceAllFacetedTags(
            const FacetId& facetId,
            const Tag& tag);

    int removeAllFacetedTags(
            const FacetId& facetId);

    /// Get all tags of a given facet sorted by score in descending order
    TagVector getFacetedTagsOrdered(
            const FacetId& facetId) const;

    /// Get the label of a single faceted tag, i.e. that
    /// occurs at most once and has no custom score. If the
    /// facet is unknown/absent an empty label is returned.
    Label getFacetedLabel(
            const FacetId& facetId) const;

    /// Get the score of a tag if present.
    ///
    /// Returns `std::nullopt` if the corresponding (facet, label)
    /// combination that serves as the key does not exist.
    std::optional<Score> getScore(
            const FacetId& facetId,
            const Label& label = Label()) const;

    enum class AggregateScoring {
        Maximum,
        Average,
    };

    // Merge the score and label of all faceted tags into
    // a single plain tag. The strings of the labels are joined
    // with a separator in between and the scores are aggregated.
    Tag mergeFacetedTags(
            const FacetId& facetId,
            AggregateScoring aggregateScoring,
            const Label::value_t& joinLabelSeparator = Label::value_t()) const;
    Label joinFacetedTagsLabel(
            const FacetId& facetId,
            const Label::value_t& joinLabelSeparator = Label::value_t()) const {
        return mergeFacetedTags(
                facetId,
                AggregateScoring::Maximum,
                joinLabelSeparator)
                .getLabel();
    }

    enum class FromJsonMode {
        Lenient,
        Strict,
    };
    static std::optional<CustomTags> fromJsonObject(
            const QJsonObject& jsonObject,
            FromJsonMode mode = FromJsonMode::Lenient);
    enum class ToJsonMode {
        Plain,
        Compact,
    };
    QJsonObject toJsonObject(
            ToJsonMode mode = ToJsonMode::Compact) const;

    enum class ParseJsonDataResult {
        Ok,
        DeserializationError,
        TransformationError,
    };
    /// Create a new instance from JSON data
    ///
    /// Returns a new instance if no error occurred. The resulting
    /// instance should be validated for consistency using `validate()`.
    static std::pair<std::optional<CustomTags>, ParseJsonDataResult> parseJsonData(
            const QByteArray& jsonData,
            FromJsonMode mode = FromJsonMode::Lenient);
    QByteArray dumpJsonData() const;
};

bool operator==(const CustomTags& lhs, const CustomTags& rhs);

inline bool operator!=(const CustomTags& lhs, const CustomTags& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const CustomTags& arg);

} // namespace tags

} // namespace library

} // namespace mixxx
