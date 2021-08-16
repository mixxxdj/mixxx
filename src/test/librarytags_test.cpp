#include <gtest/gtest.h>

#include <QJsonArray>
#include <QJsonObject>

#include "library/tags/tag.h"

namespace mixxx {

namespace library {

namespace tags {

class TagFacetTest : public testing::Test {
};

TEST_F(TagFacetTest, isEmpty) {
    ASSERT_TRUE(TagFacet().isEmpty());
    // Null string
    EXPECT_EQ(
            TagFacet(),
            TagFacet(TagFacet::value_t()));
    // Empty string
    EXPECT_EQ(
            TagFacet(),
            TagFacet(TagFacet::filterEmptyValue("")));
    // Non-empty string
    EXPECT_FALSE(TagFacet("x").isEmpty());
}

TEST_F(TagFacetTest, filterEmptyValue) {
    EXPECT_EQ(
            TagFacet::value_t(),
            TagFacet::filterEmptyValue(TagFacet::value_t()));
    EXPECT_EQ(
            TagFacet::value_t(),
            TagFacet::filterEmptyValue(""));
    EXPECT_EQ(
            TagFacet::value_t("x"),
            TagFacet::filterEmptyValue("x"));
}

TEST_F(TagFacetTest, clampValue) {
    // Clamp empty string
    EXPECT_EQ(
            TagFacet(),
            TagFacet(TagFacet::clampValue("")));
    // Clamped whitespace string
    EXPECT_EQ(
            TagFacet(),
            TagFacet(TagFacet::clampValue("  \t\n   ")));
    // Lowercase
    EXPECT_EQ(
            TagFacet("x"),
            TagFacet(TagFacet::clampValue("  \tX\n   ")));
    // Whitespace replacement
    EXPECT_EQ(
            TagFacet("xy_{+}"),
            TagFacet(TagFacet::clampValue("X y _\n{+} ")));
    // Lowercase ASCII alphabet without whitespace
    EXPECT_EQ(
            TagFacet("!\"#$%&'()*+,-./0123456789:;<=>?"
                     "@abcdefghijklmnopqrstuvwxyz[\\]^_"
                     "`abcdefghijklmnopqrstuvwxyz{|}~"),
            TagFacet(TagFacet::clampValue(
                    " !\"#$%&'()*+,-./0123456789:;<=>?"
                    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                    "`abcdefghijklmnopqrstuvwxyz{|}~")));
}

class TagLabelTest : public testing::Test {
};

TEST_F(TagLabelTest, isEmpty) {
    ASSERT_TRUE(TagLabel().isEmpty());
    // Null string
    EXPECT_EQ(
            TagLabel(),
            TagLabel(TagLabel::value_t()));
    // Empty string
    EXPECT_EQ(
            TagLabel(),
            TagLabel(TagLabel::filterEmptyValue("")));
    // Non-empty string
    EXPECT_FALSE(TagLabel("X").isEmpty());
}

TEST_F(TagLabelTest, clampEmptyValueIsValid) {
    EXPECT_TRUE(
            TagLabel::isValidValue(TagLabel::clampValue(TagLabel::value_t())));
    EXPECT_TRUE(
            TagLabel::isValidValue(TagLabel::clampValue("")));
    EXPECT_TRUE(
            TagLabel::isValidValue(TagLabel::clampValue(" ")));
}

TEST_F(TagLabelTest, filterEmptyValue) {
    EXPECT_EQ(
            TagLabel::value_t(),
            TagLabel::filterEmptyValue(TagLabel::value_t()));
    EXPECT_EQ(
            TagLabel::value_t(),
            TagLabel::filterEmptyValue(""));
    EXPECT_EQ(
            TagLabel::value_t("X"),
            TagLabel::filterEmptyValue("X"));
}

TEST_F(TagLabelTest, clampValue) {
    // Clamp empty string
    EXPECT_TRUE(
            TagLabel::isValidValue(TagLabel::clampValue("")));
    // Clamped whitespace string
    EXPECT_EQ(
            TagLabel(),
            TagLabel(TagLabel::clampValue("  \t\n   ")));
    // Trimmed
    EXPECT_EQ(
            TagLabel("X y"),
            TagLabel(TagLabel::clampValue("  \tX y\n   ")));
}

class TagTest : public testing::Test {
};

TEST_F(TagTest, fromJsonValue) {
    const char* labelText = "Label";
    const auto label = TagLabel(labelText);
    const TagScore::value_t scoreValue = 0.1357;
    const auto score = TagScore(scoreValue);
    // Empty array
    EXPECT_EQ(
            std::nullopt,
            Tag::fromJsonValue(
                    QJsonArray{}));
    // Empty label
    EXPECT_EQ(
            std::make_optional(Tag()),
            Tag::fromJsonValue(
                    ""));
    // Default score
    EXPECT_EQ(
            std::make_optional(Tag()),
            Tag::fromJsonValue(
                    TagScore::kDefaultValue));
    // Label
    EXPECT_EQ(
            std::make_optional(Tag(label)),
            Tag::fromJsonValue(
                    labelText));
    // Label as 1-element array
    EXPECT_EQ(
            std::nullopt,
            Tag::fromJsonValue(
                    QJsonArray{labelText}));
    // Score
    EXPECT_EQ(
            std::make_optional(Tag(score)),
            Tag::fromJsonValue(
                    scoreValue));
    // Score as 1-element array
    EXPECT_EQ(
            std::nullopt,
            Tag::fromJsonValue(
                    QJsonArray{scoreValue}));
    // Both label and score as array
    EXPECT_EQ(
            std::make_optional(Tag(label, score)),
            Tag::fromJsonValue(
                    QJsonArray{labelText, scoreValue}));
    // 3-element array
    EXPECT_EQ(
            std::nullopt,
            Tag::fromJsonValue(
                    QJsonArray{labelText, scoreValue, scoreValue}));
}

TEST_F(TagTest, toJsonValue) {
    const char* labelText = "Label";
    const auto label = TagLabel(labelText);
    const TagScore::value_t scoreValue = 0.1357;
    const auto score = TagScore(scoreValue);
    // Only label, no score
    EXPECT_EQ(
            QJsonValue{labelText},
            Tag(label).toJsonValue());
    // Only score, no label
    EXPECT_EQ(
            QJsonValue{TagScore::kDefaultValue},
            Tag().toJsonValue());
    EXPECT_EQ(
            QJsonValue{scoreValue},
            Tag(score).toJsonValue());
    // Both label and score
    const auto expectedJsonArray = QJsonArray{labelText, scoreValue};
    EXPECT_EQ(
            QJsonValue{expectedJsonArray},
            Tag(label, score).toJsonValue());
}

} // namespace tags

} // namespace library

} // namespace mixxx
