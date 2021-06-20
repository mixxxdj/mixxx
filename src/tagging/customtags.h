#pragma once

#include <QMap>

#include "tagging/tags.h"

namespace mixxx {

typedef QMap<TagLabel, TagScore> TagMap;
typedef QMap<TagFacet, TagMap> FacetTagMap;

class CustomTags final {
    MIXXX_DECL_PROPERTY(FacetTagMap, facetedTags, FacetedTags)

  public:
    // Predefined labels for identifying the source or owner of a score
    // for predefined or shared facets, e.g. for ratings.
    static const TagLabel kLabelMixxxOrg;

    // Predefined facets for multi-valued attributes that override
    // the single-valued track properties. Custom transformations
    // into both directions are possible by defining a mapping, i.e.
    // using a custom separator to split the single-valued attribute
    // into multiple values and to join multiple values into a single
    // values.
    static const TagFacet kFacetDecade;
    static const TagFacet kFacetEthno;
    static const TagFacet kFacetGenre;
    static const TagFacet kFacetLanguage;
    static const TagFacet kFacetMood;
    static const TagFacet kFacetVibe;

    // Predefined musical or audio feature scores (as of Spotify/EchoNest).
    // A label is optional and could be used for identifying the source of
    // the score, e.g. kLabelMixxxOrg (see below).
    //
    // The score of the facet kFacetRating captures a user's subjective
    // like or dislike level. The label is used for discrimination and may
    // refer to an organization (e.g. kLabelMixxxOrg, see below) or could
    // contain the personal e-mail address of the owner if available.
    //
    // The combination of kFacetArousal and kFacetValence could
    // be used for classifying emotion (= mood) according to Thayer's
    // arousel-valence emotion plane.
    static const TagFacet kFacetRating;
    static const TagFacet kFacetArousal; // for emotion classification
    static const TagFacet kFacetValence; // for emotion classification
    static const TagFacet kFacetAcousticness;
    static const TagFacet kFacetDanceability;
    static const TagFacet kFacetEnergy;
    static const TagFacet kFacetInstrumentalness;
    static const TagFacet kFacetLiveness;
    static const TagFacet kFacetPopularity;
    static const TagFacet kFacetSpeechiness;

    // Some facets reserved for future use cases (aoide) that are
    // not available as custom tags in Mixxx.
    static const TagFacet kReservedFacetAcoustid;
    static const TagFacet kReservedFacetComment;
    static const TagFacet kReservedFacetGrouping;
    static const TagFacet kReservedFacetISRC;
    static const TagFacet kReservedFacetMixxx;
    static const TagFacet kReservedFacetMusicBrainz;

    static const QList<TagFacet> kReservedFacets;

    static bool isReservedFacet(
            const TagFacet& facet) {
        return kReservedFacets.contains(facet);
    }

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

} // namespace mixxx
