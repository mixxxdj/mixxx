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
    const auto facet = TagFacet(QStringLiteral("facet"));
    const auto label = TagLabel(QStringLiteral("Label"));

    CustomTags customTags;
    EXPECT_TRUE(customTags.isEmpty());
    EXPECT_EQ(0, customTags.countTags(label));
    EXPECT_EQ(0, customTags.countTags(label, TagFacet()));
    EXPECT_EQ(0, customTags.countTags(label, facet));
    EXPECT_EQ(0, customTags.countFacetedTags(facet));

    customTags.addOrReplaceTag(Tag(label));
    EXPECT_FALSE(customTags.isEmpty());
    EXPECT_EQ(1, customTags.countTags(label));
    EXPECT_EQ(1, customTags.countTags(label, TagFacet()));
    EXPECT_EQ(0, customTags.countTags(label, facet));
    EXPECT_EQ(0, customTags.countFacetedTags(facet));

    customTags.removeTag(label);
    EXPECT_TRUE(customTags.isEmpty());
    EXPECT_EQ(0, customTags.countTags(label));
    EXPECT_EQ(0, customTags.countTags(label, TagFacet()));
    EXPECT_EQ(0, customTags.countTags(label, facet));
    EXPECT_EQ(0, customTags.countFacetedTags(facet));

    customTags.addOrReplaceTag(Tag(label), facet);
    EXPECT_FALSE(customTags.isEmpty());
    EXPECT_EQ(0, customTags.countTags(label));
    EXPECT_EQ(0, customTags.countTags(label, TagFacet()));
    EXPECT_EQ(1, customTags.countTags(label, facet));
    EXPECT_EQ(1, customTags.countFacetedTags(facet));

    customTags.removeTag(label, facet);
    EXPECT_TRUE(customTags.isEmpty());
    EXPECT_EQ(0, customTags.countTags(label));
    EXPECT_EQ(0, customTags.countTags(label, TagFacet()));
    EXPECT_EQ(0, customTags.countTags(label, facet));
    EXPECT_EQ(0, customTags.countFacetedTags(facet));
}

TEST_F(CustomTagsTest, isEmptyWithEmptyFacetedSlot) {
    const auto facet = TagFacet(QStringLiteral("facet"));

    CustomTags customTags;
    ASSERT_EQ(0, customTags.getFacetedTags().size());
    ASSERT_TRUE(customTags.isEmpty());

    // Add an empty placeholder slot just for the facet
    customTags.refFacetedTags(facet);

    // Verify that an empty slot for faceted tags does
    // not affect the emptiness check.
    EXPECT_EQ(1, customTags.getFacetedTags().size());
    EXPECT_TRUE(customTags.isEmpty());

    // Add an empty placeholder slot with an empty facet
    customTags.refFacetedTags(TagFacet());

    // Verify that an empty slot for an empty facet does
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

    const auto facet1 = TagFacet(QStringLiteral("facet1"));
    const auto facet2 = TagFacet(QStringLiteral("facet2"));
    const auto facet3 = TagFacet(QStringLiteral("facet3"));

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
            decoded->getFacetedTags().value(TagFacet()).value(label1));
    EXPECT_EQ(
            TagScore(0.5),
            decoded->getFacetedTags().value(TagFacet()).value(label2));

    // Faceted tags
    EXPECT_EQ(2, decoded->countFacetedTags(facet1));
    EXPECT_EQ(2, decoded->countFacetedTags(facet2));
    EXPECT_EQ(0, decoded->countFacetedTags(facet3));
    const auto facet1TagsOrdered = TagVector{
            Tag(label1),
            Tag(TagScore(0.2))};
    EXPECT_EQ(
            facet1TagsOrdered,
            decoded->getFacetedTagsOrdered(facet1));
    const auto facet2TagsOrdered = TagVector{
            Tag(),
            Tag(label2, TagScore(0.4))};
    EXPECT_EQ(
            facet2TagsOrdered,
            decoded->getFacetedTagsOrdered(facet2));

    // Compaction
    decoded->compact();
    // facet3 has disappeared
    EXPECT_EQ(3, decoded->getFacetedTags().size());
}

} // namespace tags

} // namespace library

} // namespace mixxx
