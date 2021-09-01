#include "library/tags/facets.h"

#include <gtest/gtest.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace mixxx {

namespace library {

namespace tags {

class FacetsTest : public testing::Test {
};

TEST_F(FacetsTest, isEmptyAndCount) {
    const auto facetId = FacetId(QStringLiteral("facet-id"));
    const auto label = Label(QStringLiteral("Label"));

    Facets facets;
    EXPECT_TRUE(facets.isEmpty());
    EXPECT_EQ(0, facets.countTagsLabeled(label));
    EXPECT_EQ(0, facets.countTagsLabeled(label, FacetId{}));
    EXPECT_EQ(0, facets.countTagsLabeled(label, facetId));
    EXPECT_EQ(0, facets.countTags(facetId));

    facets.addOrUpdateTag(Tag{label});
    EXPECT_FALSE(facets.isEmpty());
    EXPECT_EQ(1, facets.countTagsLabeled(label));
    EXPECT_EQ(1, facets.countTagsLabeled(label, FacetId{}));
    EXPECT_EQ(0, facets.countTagsLabeled(label, facetId));
    EXPECT_EQ(0, facets.countTags(facetId));

    facets.removeTagLabeled(label);
    EXPECT_TRUE(facets.isEmpty());
    EXPECT_EQ(0, facets.countTagsLabeled(label));
    EXPECT_EQ(0, facets.countTagsLabeled(label, FacetId{}));
    EXPECT_EQ(0, facets.countTagsLabeled(label, facetId));
    EXPECT_EQ(0, facets.countTags(facetId));

    facets.addOrUpdateTag(Tag{label}, facetId);
    EXPECT_FALSE(facets.isEmpty());
    EXPECT_EQ(0, facets.countTagsLabeled(label));
    EXPECT_EQ(0, facets.countTagsLabeled(label, FacetId{}));
    EXPECT_EQ(1, facets.countTagsLabeled(label, facetId));
    EXPECT_EQ(1, facets.countTags(facetId));

    facets.removeTagLabeled(label, facetId);
    EXPECT_TRUE(facets.isEmpty());
    EXPECT_EQ(0, facets.countTagsLabeled(label));
    EXPECT_EQ(0, facets.countTagsLabeled(label, FacetId{}));
    EXPECT_EQ(0, facets.countTagsLabeled(label, facetId));
    EXPECT_EQ(0, facets.countTags(facetId));
}

TEST_F(FacetsTest, isEmptyWithEmptyFacetedSlot) {
    const auto facetId = FacetId(QStringLiteral("facet"));

    Facets facets;
    ASSERT_EQ(0, facets.countFacets());
    ASSERT_TRUE(facets.isEmpty());

    // Add an empty placeholder slot just for the facetId
    facets.refTags(facetId);

    // Verify that an empty slot for faceted tags does
    // not affect the emptiness check.
    EXPECT_EQ(1, facets.countFacets());
    EXPECT_TRUE(facets.isEmpty());

    // Add an empty placeholder slot with an empty facetId
    facets.refTags(FacetId());

    // Verify that an empty slot for an empty facetId does
    // not affect the emptiness check.
    EXPECT_EQ(2, facets.countFacets());
    EXPECT_TRUE(facets.isEmpty());
}

TEST_F(FacetsTest, encodeDecodeJsonRoundtripEmpty) {
    // Encode default/empty JSON
    EXPECT_EQ(QJsonObject{},
            Facets().toJsonObject(Facets::ToJsonMode::Plain));
    EXPECT_EQ(QJsonObject{},
            Facets().toJsonObject(Facets::ToJsonMode::Compact));
    EXPECT_EQ(QJsonObject{},
            Facets().toJsonObject(/*default*/));

    // JSON with empty arrays
    const auto facetsJson = QJsonObject{
            {
                    "",
                    QJsonArray{},
            },
            {
                    "facet",
                    QJsonArray{},
            },
    };

    // Decode JSON with empty arrays
    const QByteArray jsonData =
            QJsonDocument{facetsJson}.toJson(QJsonDocument::Compact);
    const auto jsonDoc =
            QJsonDocument::fromJson(jsonData);
    std::optional<Facets> decoded =
            Facets::fromJsonObject(jsonDoc.object());
    ASSERT_TRUE(static_cast<bool>(decoded));
    ASSERT_TRUE(decoded->validate());
    // Empty arrays shall be preserved by decoding
    EXPECT_EQ(2, decoded->countFacets());

    // Re-encode JSON with empty arrays
    EXPECT_EQ(facetsJson,
            decoded->toJsonObject(Facets::ToJsonMode::Plain));
    EXPECT_EQ(QJsonObject{},
            decoded->toJsonObject(Facets::ToJsonMode::Compact));
    EXPECT_EQ(QJsonObject{},
            decoded->toJsonObject(/*default*/));

    // Re-encode JSON with empty arrays after compacting
    decoded->compact();
    EXPECT_EQ(QJsonObject{},
            decoded->toJsonObject(Facets::ToJsonMode::Plain));
}

TEST_F(FacetsTest, encodeDecodeJsonRoundtrip) {
    const auto facetsJson = QJsonObject{
            {
                    "facet1",
                    QJsonArray{
                            QJsonValue{0.2},       // empty label
                            QJsonValue{"Label 1"}, // score = 1.0
                    },
            },
            {
                    "facet2",
                    QJsonArray{
                            QJsonValue{1.0},
                            QJsonArray{QJsonValue{"Label 2"}, QJsonValue{0.4}},
                    },
            },
            {
                    "facet3",
                    QJsonArray{},
            },
            {
                    "",
                    QJsonArray{
                            QJsonValue{"Label 1"},
                            QJsonArray{QJsonValue{"Label 2"}, QJsonValue{0.5}},
                    },
            },
    };

    const auto facetId1 = FacetId(QStringLiteral("facet1"));
    const auto facetId2 = FacetId(QStringLiteral("facet2"));
    const auto facetId3 = FacetId(QStringLiteral("facet3"));

    const auto label1 = Label(QStringLiteral("Label 1"));
    const auto label2 = Label(QStringLiteral("Label 2"));

    // Decode JSON
    const QByteArray jsonData =
            QJsonDocument{facetsJson}.toJson(QJsonDocument::Compact);
    const auto jsonDoc =
            QJsonDocument::fromJson(jsonData);
    std::optional<Facets> decoded =
            Facets::fromJsonObject(jsonDoc.object());
    ASSERT_TRUE(static_cast<bool>(decoded));
    ASSERT_TRUE(decoded->validate());

    // All tags (plain + 2 facets with tags + 1 facet with an empty slot)
    EXPECT_EQ(4, decoded->countFacets());

    // Plain tags
    EXPECT_EQ(2, decoded->countTags());
    EXPECT_EQ(1, decoded->countTagsLabeled(label1));
    EXPECT_EQ(1, decoded->countTagsLabeled(label2));
    EXPECT_EQ(
            Score{},
            decoded->getTagScore(label1));
    EXPECT_EQ(
            Score{0.5},
            decoded->getTagScore(label2));

    // Faceted tags
    EXPECT_EQ(2, decoded->countTags(facetId1));
    EXPECT_EQ(2, decoded->countTags(facetId2));
    EXPECT_EQ(0, decoded->countTags(facetId3));
    const auto facetId1TagsOrdered = TagVector{
            Tag(label1),
            Tag(Score(0.2))};
    EXPECT_EQ(
            facetId1TagsOrdered,
            decoded->collectTagsOrdered(
                    Facets::ScoreOrdering::Descending,
                    facetId1));
    const auto facetId2TagsOrdered = TagVector{
            Tag(),
            Tag(label2, Score(0.4))};
    EXPECT_EQ(
            facetId2TagsOrdered,
            decoded->collectTagsOrdered(
                    Facets::ScoreOrdering::Descending,
                    facetId2));

    // Compaction
    decoded->compact();
    // facet3 has disappeared
    EXPECT_EQ(3, decoded->countFacets());
}

} // namespace tags

} // namespace library

} // namespace mixxx
