#include <gtest/gtest.h>

#include <QJsonArray>
#include <QJsonObject>

#include "library/tags/tag.h"
#include "library/tags/tagfacetid.h"

namespace mixxx {

namespace library {

namespace tags {

class TagFacetIdTest : public testing::Test {
};

TEST_F(TagFacetIdTest, isEmpty) {
    ASSERT_TRUE(TagFacetId().isEmpty());
    // Null string
    EXPECT_EQ(
            TagFacetId(),
            TagFacetId(TagFacetId::value_t()));
    // Empty string
    EXPECT_EQ(
            TagFacetId(),
            TagFacetId(TagFacetId::filterEmptyValue("")));
    // Non-empty string
    EXPECT_FALSE(TagFacetId("x").isEmpty());
}

TEST_F(TagFacetIdTest, filterEmptyValue) {
    EXPECT_EQ(
            TagFacetId::value_t(),
            TagFacetId::filterEmptyValue(TagFacetId::value_t()));
    EXPECT_EQ(
            TagFacetId::value_t(),
            TagFacetId::filterEmptyValue(""));
    EXPECT_EQ(
            TagFacetId::value_t("x"),
            TagFacetId::filterEmptyValue("x"));
}

TEST_F(TagFacetIdTest, convertIntoValidValue) {
    // Clamp empty string
    EXPECT_EQ(
            TagFacetId::value_t{},
            TagFacetId::convertIntoValidValue(""));
    // Clamped whitespace string
    EXPECT_EQ(
            TagFacetId::value_t{},
            TagFacetId::convertIntoValidValue("  \t\n   "));
    // Lowercase
    EXPECT_EQ(
            TagFacetId::value_t{"x"},
            TagFacetId::convertIntoValidValue("  \tX\n   "));
    // Whitespace replacement
    EXPECT_EQ(
            TagFacetId::value_t{"xy_[+]"},
            TagFacetId::convertIntoValidValue("X y _\n[+] "));
    // Valid characters without whitespace
    EXPECT_EQ(
            TagFacetId::value_t{TagFacetId::kAlphabet},
            TagFacetId::convertIntoValidValue(
                    TagFacetId::value_t{TagFacetId::kAlphabet}));
    EXPECT_EQ(
            TagFacetId::value_t{"+-./0123456789"
                                "@abcdefghijklmnopqrstuvwxyz[]_"
                                "abcdefghijklmnopqrstuvwxyz"},
            TagFacetId::convertIntoValidValue(
                    "\t!\"#$%&'()*+,-./0123456789:;<=>?"
                    " @ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_"
                    " `abcdefghijklmnopqrstuvwxyz{|}~\n"));
}

TEST_F(TagFacetIdTest, isValidValue) {
    EXPECT_TRUE(TagFacetId::isValidValue(QString()));
    EXPECT_TRUE(TagFacetId::isValidValue(QStringLiteral("facet@mixxx.org/simple-test[1]")));
    EXPECT_FALSE(TagFacetId::isValidValue(QStringLiteral("Uppercase")));
    EXPECT_FALSE(TagFacetId::isValidValue(
            QStringLiteral("lower-case_with_digit_1_and_whitespace ")));
    EXPECT_FALSE(TagFacetId::isValidValue(QLatin1String("")));
    EXPECT_FALSE(TagFacetId::isValidValue(QStringLiteral(" ")));
}

TEST_F(TagFacetIdTest, convertedEmptyValueIsValid) {
    EXPECT_TRUE(
            TagFacetId::isValidValue(TagFacetId::convertIntoValidValue(TagFacetId::value_t{})));
    EXPECT_TRUE(
            TagFacetId::isValidValue(TagFacetId::convertIntoValidValue("")));
    EXPECT_TRUE(
            TagFacetId::isValidValue(TagFacetId::convertIntoValidValue(" ")));
    EXPECT_TRUE(
            TagFacetId::isValidValue(TagFacetId::convertIntoValidValue(" \t \n \r ")));
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

TEST_F(TagLabelTest, convertIntoValidValue) {
    // Clamp empty string
    EXPECT_TRUE(
            TagLabel::isValidValue(TagLabel::convertIntoValidValue("")));
    // Clamped whitespace string
    EXPECT_EQ(
            TagLabel::value_t{},
            TagLabel::convertIntoValidValue("  \t\n   "));
    // Trimmed
    EXPECT_EQ(
            TagLabel::value_t{"X y"},
            TagLabel::convertIntoValidValue("  \tX y\n   "));
}

TEST_F(TagLabelTest, isValidValue) {
    EXPECT_TRUE(TagLabel::isValidValue(QString()));
    EXPECT_TRUE(TagLabel::isValidValue(QStringLiteral("lower-case_with_digit_1")));
    EXPECT_TRUE(TagLabel::isValidValue(QStringLiteral("With\\backslash")));
    EXPECT_TRUE(TagLabel::isValidValue(QStringLiteral("Uppercase with\twhitespace")));
    EXPECT_FALSE(TagLabel::isValidValue(QStringLiteral(" With leading space")));
    EXPECT_FALSE(TagLabel::isValidValue(QStringLiteral("With trailing space\n")));
    EXPECT_FALSE(TagLabel::isValidValue(QLatin1String("")));
    EXPECT_FALSE(TagLabel::isValidValue(QStringLiteral(" ")));
}

TEST_F(TagLabelTest, convertedEmptyValueIsValid) {
    EXPECT_TRUE(
            TagLabel::isValidValue(TagLabel::convertIntoValidValue(TagLabel::value_t{})));
    EXPECT_TRUE(
            TagLabel::isValidValue(TagLabel::convertIntoValidValue("")));
    EXPECT_TRUE(
            TagLabel::isValidValue(TagLabel::convertIntoValidValue(" ")));
    EXPECT_TRUE(
            TagLabel::isValidValue(TagFacetId::convertIntoValidValue(" \t \n \r ")));
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
