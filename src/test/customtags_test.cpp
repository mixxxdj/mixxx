#include "library/tags/customtags.h"

#include <gtest/gtest.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace mixxx {

namespace library {

namespace tags {

class CustomTagsTest : public testing::Test {
};

TEST_F(CustomTagsTest, isEmptyAndCount) {
    const auto facetId = TagFacetId(QStringLiteral("facet-id"));
    const auto label = TagLabel(QStringLiteral("Label"));

    CustomTags customTags;
    EXPECT_TRUE(customTags.isEmpty());
    EXPECT_EQ(0, customTags.countTags(label));
    EXPECT_EQ(0, customTags.countTags(label, TagFacetId()));
    EXPECT_EQ(0, customTags.countTags(label, facetId));
    EXPECT_EQ(0, customTags.countFacetedTags(facetId));

    customTags.addOrReplaceTag(Tag(label));
    EXPECT_FALSE(customTags.isEmpty());
    EXPECT_EQ(1, customTags.countTags(label));
    EXPECT_EQ(1, customTags.countTags(label, TagFacetId()));
    EXPECT_EQ(0, customTags.countTags(label, facetId));
    EXPECT_EQ(0, customTags.countFacetedTags(facetId));

    customTags.removeTag(label);
    EXPECT_TRUE(customTags.isEmpty());
    EXPECT_EQ(0, customTags.countTags(label));
    EXPECT_EQ(0, customTags.countTags(label, TagFacetId()));
    EXPECT_EQ(0, customTags.countTags(label, facetId));
    EXPECT_EQ(0, customTags.countFacetedTags(facetId));

    customTags.addOrReplaceTag(Tag(label), facetId);
    EXPECT_FALSE(customTags.isEmpty());
    EXPECT_EQ(0, customTags.countTags(label));
    EXPECT_EQ(0, customTags.countTags(label, TagFacetId()));
    EXPECT_EQ(1, customTags.countTags(label, facetId));
    EXPECT_EQ(1, customTags.countFacetedTags(facetId));

    customTags.removeTag(label, facetId);
    EXPECT_TRUE(customTags.isEmpty());
    EXPECT_EQ(0, customTags.countTags(label));
    EXPECT_EQ(0, customTags.countTags(label, TagFacetId()));
    EXPECT_EQ(0, customTags.countTags(label, facetId));
    EXPECT_EQ(0, customTags.countFacetedTags(facetId));
}

TEST_F(CustomTagsTest, isEmptyWithEmptyFacetedSlot) {
    const auto facetId = TagFacetId(QStringLiteral("facet"));

    CustomTags customTags;
    ASSERT_EQ(0, customTags.getFacetedTags().size());
    ASSERT_TRUE(customTags.isEmpty());

    // Add an empty placeholder slot just for the facetId
    customTags.refFacetedTags(facetId);

    // Verify that an empty slot for faceted tags does
    // not affect the emptiness check.
    EXPECT_EQ(1, customTags.getFacetedTags().size());
    EXPECT_TRUE(customTags.isEmpty());

    // Add an empty placeholder slot with an empty facetId
    customTags.refFacetedTags(TagFacetId());

    // Verify that an empty slot for an empty facetId does
    // not affect the emptiness check.
    EXPECT_EQ(2, customTags.getFacetedTags().size());
    EXPECT_TRUE(customTags.isEmpty());
}

TEST_F(CustomTagsTest, encodeDecodeJsonRoundtripEmpty) {
    // Encode default/empty JSON
    EXPECT_EQ(QJsonObject{},
            CustomTags().toJsonObject(CustomTags::ToJsonMode::Plain));
    EXPECT_EQ(QJsonObject{},
            CustomTags().toJsonObject(CustomTags::ToJsonMode::Compact));
    EXPECT_EQ(QJsonObject{},
            CustomTags().toJsonObject(/*default*/));

    // JSON with empty arrays
    const auto customTagsJson = QJsonObject{
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
            QJsonDocument{customTagsJson}.toJson(QJsonDocument::Compact);
    const auto jsonDoc =
            QJsonDocument::fromJson(jsonData);
    std::optional<CustomTags> decoded =
            CustomTags::fromJsonObject(jsonDoc.object());
    ASSERT_TRUE(static_cast<bool>(decoded));
    ASSERT_TRUE(decoded->validate());
    // Empty arrays shall be preserved by decoding
    EXPECT_EQ(2, decoded->getFacetedTags().size());

    // Re-encode JSON with empty arrays
    EXPECT_EQ(customTagsJson,
            decoded->toJsonObject(CustomTags::ToJsonMode::Plain));
    EXPECT_EQ(QJsonObject{},
            decoded->toJsonObject(CustomTags::ToJsonMode::Compact));
    EXPECT_EQ(QJsonObject{},
            decoded->toJsonObject(/*default*/));

    // Re-encode JSON with empty arrays after compacting
    decoded->compact();
    EXPECT_EQ(QJsonObject{},
            decoded->toJsonObject(CustomTags::ToJsonMode::Plain));
}

TEST_F(CustomTagsTest, encodeDecodeJsonRoundtrip) {
    const auto customTagsJson = QJsonObject{
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

    const auto facetId1 = TagFacetId(QStringLiteral("facet1"));
    const auto facetId2 = TagFacetId(QStringLiteral("facet2"));
    const auto facetId3 = TagFacetId(QStringLiteral("facet3"));

    const auto label1 = TagLabel(QStringLiteral("Label 1"));
    const auto label2 = TagLabel(QStringLiteral("Label 2"));

    // Decode JSON
    const QByteArray jsonData =
            QJsonDocument{customTagsJson}.toJson(QJsonDocument::Compact);
    const auto jsonDoc =
            QJsonDocument::fromJson(jsonData);
    std::optional<CustomTags> decoded =
            CustomTags::fromJsonObject(jsonDoc.object());
    ASSERT_TRUE(static_cast<bool>(decoded));
    ASSERT_TRUE(decoded->validate());

    // All tags (plain + 2 facets with tags + 1 facet with an empty slot)
    EXPECT_EQ(4, decoded->getFacetedTags().size());

    // Plain tags
    EXPECT_EQ(2, decoded->countTags());
    EXPECT_EQ(1, decoded->countTags(label1));
    EXPECT_EQ(1, decoded->countTags(label2));
    EXPECT_EQ(
            TagScore(),
            decoded->getFacetedTags().value(TagFacetId()).value(label1));
    EXPECT_EQ(
            TagScore(0.5),
            decoded->getFacetedTags().value(TagFacetId()).value(label2));

    // Faceted tags
    EXPECT_EQ(2, decoded->countFacetedTags(facetId1));
    EXPECT_EQ(2, decoded->countFacetedTags(facetId2));
    EXPECT_EQ(0, decoded->countFacetedTags(facetId3));
    const auto facetId1TagsOrdered = TagVector{
            Tag(label1),
            Tag(TagScore(0.2))};
    EXPECT_EQ(
            facetId1TagsOrdered,
            decoded->getFacetedTagsOrdered(facetId1));
    const auto facetId2TagsOrdered = TagVector{
            Tag(),
            Tag(label2, TagScore(0.4))};
    EXPECT_EQ(
            facetId2TagsOrdered,
            decoded->getFacetedTagsOrdered(facetId2));

    // Compaction
    decoded->compact();
    // facet3 has disappeared
    EXPECT_EQ(3, decoded->getFacetedTags().size());
}

} // namespace tags

} // namespace library

} // namespace mixxx
