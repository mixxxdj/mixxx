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

/// Set of both faceted and non-faceted tags.
///
/// Each facet is represented by a 3-tuple (triple) of a *facet (`FacetId`),
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
/// The tuple (`facet`, `label`) defines a uniqueness constraint among all facets,
/// i.e. within an instance of `Facets` which is basically two nested maps. The
/// 1st key is `facet` (nullable) and the 2nd key is `label` (nullable).
///
/// Plain, non-faceted tags are accessed by an empty facet. For this purpose
/// most operations have an optional `facetId` parameter that could be omitted
/// for accessing the plain, non-faceted tags.
class Facets final {
  public:
    Facets() = default;
    Facets(Facets&&) = default;
    Facets(const Facets&) = default;
    Facets& operator=(Facets&&) = default;
    Facets& operator=(const Facets&) = default;

    /// Check for consistency
    bool validate() const;

    /// Purge all empty and unused map entries, i.e. facets
    /// without any tags.
    void compact();

    FacetTagMap::const_iterator begin() const {
        return m_impl.begin();
    }
    FacetTagMap::const_iterator end() const {
        return m_impl.end();
    }

    bool isEmpty() const;

    int countAllTags() const;

    int countFacets() const {
        return m_impl.size();
    }

    bool containsFacet(
            const FacetId& facetId) const {
        return m_impl.contains(facetId);
    }

    /// Count all tags
    int countTags(
            const FacetId& facetId = FacetId{}) const;

    /// Access all tags (mutable)
    ///
    /// This operation will create a new, empty entry in the internal
    /// map if the given facet does not exist.
    TagMap& refTags(
            const FacetId& facetId = FacetId{}) {
        return m_impl[facetId];
    }

    int removeTags(
            const FacetId& facetId = FacetId{});

    /// Checks if a tag with the given label exists
    bool containsTagLabeled(
            const Label& label,
            const FacetId& facetId = FacetId{}) const;

    /// Count all tags by the given label
    ///
    /// Should return either 0 or 1.
    int countTagsLabeled(
            const Label& label,
            const FacetId& facetId = FacetId{}) const;

    /// Remove a tag by label
    ///
    /// Returns true if a tag has been found and removed.
    bool removeTagLabeled(
            const Label& label,
            const FacetId& facetId = FacetId{});

    /// Get the singular tag of a facet
    ///
    /// Triggers a debug assertion if the facet has more than 1 tag.
    ///
    /// Returns `std::nullopt` if the facet has no tags.
    std::optional<Tag> getSingleTag(
            const FacetId& facetId = FacetId{}) const;

    /// Get the singular tag label of a facet
    ///
    /// Triggers a debug assertion if the facet has more than 1 tag.
    ///
    /// Returns `std::nullopt` if the facet has no tags.
    std::optional<Label> getSingleTagLabel(
            const FacetId& facetId = FacetId{}) const {
        const auto tag = getSingleTag(facetId);
        if (tag) {
            return tag->getLabel();
        } else {
            return std::nullopt;
        }
    }

    /// Get the score of a tag if present.
    ///
    /// Returns `std::nullopt` if the corresponding (facet, label)
    /// combination that serves as the key does not exist.
    std::optional<Score> getTagScore(
            const Label& label,
            const FacetId& facetId = FacetId{}) const;

    /// Collect all tags of a given facet in no particular order
    ///
    /// Returns an empty vector if no tags exist.
    TagVector collectTags(
            const FacetId& facetId = FacetId{}) const;

    enum class ScoreOrdering {
        Ascending,
        Descending,
    };
    /// Collect all tags of a given facet ordered by score
    ///
    /// Returns an empty vector if no tags exist.
    TagVector collectTagsOrdered(
            ScoreOrdering scoreOrdering,
            const FacetId& facetId = FacetId{}) const;

    /// Add an (empty) entry for the given facet if it does not exist yet.
    ///
    /// Existing entries are not affected.
    void addOrIgnoreFacet(
            const FacetId& facetId = FacetId{}) {
        refTags(facetId);
        DEBUG_ASSERT(containsFacet(facetId));
    }

    /// Add the tag or update the score of an existing tag
    ///
    /// Returns true if (probably) modified and false if unmodified.
    bool addOrUpdateTag(
            const Tag& tag,
            const FacetId& facetId = FacetId{});

    /// Additive merge operation
    ///
    /// Returns the number of tags that have been modified, i.e.
    /// both added and updated.
    int addOrUpdateAllTags(
            const Facets& facets);

    /// Add the tag or update the score of an existing tag
    ///
    /// Either add a new facet with the given tags or replace the tags
    /// of an existing facet.
    ///
    /// Returns true if (probably) modified and false if unmodified.
    void addOrReplaceFacet(
            const TagMap& tags,
            const FacetId& facetId = FacetId{});

    /// Replacing merge operation
    ///
    /// For each facet: Either add a new facet together with its tags
    /// or replace the tags of an existing facet with the new tags.
    ///
    /// Returns true if (probably) modified and false if unmodified.
    void addOrReplaceAllFacets(
            const Facets& facets);

    /// Replace all existing tags of the facet with a single
    /// tag or insert a new facet with a single tag.
    ///
    /// Returns an iterator to the inserted or updated tag.
    FacetTagMap::iterator addOrReplaceTagsWithSingleTag(
            const Tag& tag,
            const FacetId& facetId = FacetId{});

    enum class AggregateScoring {
        Maximum,
        Average,
    };

    /// Merge the scores and labels of all tags into a single tag.
    ///
    /// The strings of the labels are joined with a separator in
    /// between and the scores are aggregated.
    Tag mergeTags(
            const Label::value_t& joinLabelSeparator,
            AggregateScoring aggregateScoring,
            const FacetId& facetId = FacetId{}) const;

    /// Merge the labels of all tags into a single label.
    ///
    /// The strings of the labels are joined with a separator in
    /// between.
    Label joinTagsLabel(
            const Label::value_t& joinLabelSeparator,
            const FacetId& facetId = FacetId{}) const {
        return mergeTags(
                joinLabelSeparator,
                AggregateScoring::Maximum, // aggregated score will be discarded
                facetId)
                .getLabel();
    }

    enum class FromJsonMode {
        Lenient,
        Strict,
    };
    static std::optional<Facets> fromJsonObject(
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
    static std::pair<std::optional<Facets>, ParseJsonDataResult> parseJsonData(
            const QByteArray& jsonData,
            FromJsonMode mode = FromJsonMode::Lenient);
    QByteArray dumpJsonData() const;

    friend bool operator==(const Facets& lhs, const Facets& rhs);

    friend QDebug operator<<(QDebug dbg, const Facets& arg);

  private:
    FacetTagMap m_impl;
};

inline bool operator!=(const Facets& lhs, const Facets& rhs) {
    return !(lhs == rhs);
}

} // namespace tags

} // namespace library

} // namespace mixxx
