#pragma once

#include <QMap>

#include "library/tags/tag.h"

namespace mixxx {

namespace library {

namespace tags {

typedef QMap<TagLabel, TagScore> TagMap;
typedef QMap<TagFacet, TagMap> FacetTagMap;

/// Custom tags
///
/// Each custom tag is represented by a triple: a facet, a label, and a score:
///
///   - The *facet* is a lowercase, non-empty ASCII string without whitespace
///   - The *label* is non-empty free text
///   - The *score* is a floating-point value in the interval [0.0, 1.0]
///
/// Both *facet* and *label* are optional (= nullable), but at least one of them must be present.
/// The *score* is mandatory and is supposed to be interpreted as a *weight*, *degree of cohesion*,
/// or *normalized level* with a default value of 1.0.
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
            const TagFacet& facet) const {
        return getFacetedTags().contains(facet);
    }
    /// Add an (empty) entry for the given facet if it does not exist yet.
    ///
    /// Existing entries are not affected.
    void touchFacet(
            const TagFacet& facet) {
        refFacetedTags()[facet];
        DEBUG_ASSERT(containsFacet(facet));
    }

    bool containsTag(
            const TagLabel& label,
            const TagFacet& facet = TagFacet()) const;
    int countTags(
            const TagLabel& label,
            const TagFacet& facet = TagFacet()) const;

    int countTags() const {
        return countFacetedTags(TagFacet());
    }
    TagMap& refTags() {
        return refFacetedTags()[TagFacet()];
    }

    int countFacetedTags(
            const TagFacet& facet) const;
    TagMap& refFacetedTags(
            const TagFacet& facet) {
        return refFacetedTags()[facet];
    }

    bool addOrReplaceTag(
            const Tag& tag,
            const TagFacet& facet = TagFacet());
    bool removeTag(
            const TagLabel& label,
            const TagFacet& facet = TagFacet());

    bool addOrReplaceAllTags(
            const CustomTags& tags);

    // Get all plain tags as a list (in no particular order)
    TagVector getTags() const;

    // Replace all existing tags of this facet with a single
    // faceted tag or insert a new faceted tag.
    FacetTagMap::iterator replaceAllFacetedTags(
            const TagFacet& facet,
            const Tag& tag);
    int removeAllFacetedTags(
            const TagFacet& facet);
    // Get all tags of a given facet sorted by score in descending order
    TagVector getFacetedTagsOrdered(
            const TagFacet& facet) const;

    // Get the label of a single faceted tag, i.e. that
    // occurs at most once and has no custom score. If the
    // facet is unknown/absent an empty label is returned.
    TagLabel getFacetedTagLabel(
            const TagFacet& facet) const;

    // Get the score of a tag if present.
    std::optional<TagScore> getTagScore(
            const TagFacet& facet,
            const TagLabel& label = TagLabel()) const;

    enum class AggregateScoring {
        Maximum,
        Average,
    };

    // Merge the score and label of all faceted tags into
    // a single plain tag. The strings of the labels are joined
    // with a separator in between and the scores are aggregated.
    Tag mergeFacetedTags(
            const TagFacet& facet,
            AggregateScoring aggregateScoring,
            const TagLabel::value_t& joinLabelSeparator = TagLabel::value_t()) const;
    TagLabel joinFacetedTagsLabel(
            const TagFacet& facet,
            const TagLabel::value_t& joinLabelSeparator = TagLabel::value_t()) const {
        return mergeFacetedTags(
                facet,
                AggregateScoring::Maximum,
                joinLabelSeparator)
                .getLabel();
    }

    static std::optional<CustomTags> fromJsonObject(
            const QJsonObject& jsonObject,
            bool strict = false);
    enum class ToJsonMode {
        Plain,
        Compact,
    };
    QJsonObject toJsonObject(
            ToJsonMode mode = ToJsonMode::Compact) const;

    static std::optional<CustomTags> parseJsonData(
            const QByteArray& jsonData);
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
