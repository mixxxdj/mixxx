#include <gtest/gtest.h>

#include <QJsonArray>
#include <QJsonObject>

#include "library/tags/facetid.h"
#include "library/tags/tag.h"

namespace mixxx {

namespace library {

namespace tags {

class FacetIdTest : public testing::Test {
};

TEST_F(FacetIdTest, isEmpty) {
    ASSERT_TRUE(FacetId().isEmpty());
    // Null string
    EXPECT_EQ(
            FacetId(),
            FacetId(FacetId::value_t()));
    // Empty string
    EXPECT_EQ(
            FacetId(),
            FacetId(FacetId::filterEmptyValue("")));
    // Non-empty string
    EXPECT_FALSE(FacetId("x").isEmpty());
}

TEST_F(FacetIdTest, filterEmptyValue) {
    EXPECT_EQ(
            FacetId::value_t(),
            FacetId::filterEmptyValue(FacetId::value_t()));
    EXPECT_EQ(
            FacetId::value_t(),
            FacetId::filterEmptyValue(""));
    EXPECT_EQ(
            FacetId::value_t("x"),
            FacetId::filterEmptyValue("x"));
}

TEST_F(FacetIdTest, convertIntoValidValue) {
    // Clamp empty string
    EXPECT_EQ(
            FacetId::value_t{},
            FacetId::convertIntoValidValue(""));
    // Clamped whitespace string
    EXPECT_EQ(
            FacetId::value_t{},
            FacetId::convertIntoValidValue("  \t\n   "));
    // Lowercase
    EXPECT_EQ(
            FacetId::value_t{"x"},
            FacetId::convertIntoValidValue("  \tX\n   "));
    // Whitespace replacement
    EXPECT_EQ(
            FacetId::value_t{"xy_[+]"},
            FacetId::convertIntoValidValue("X y _\n[+] "));
    // Valid characters without whitespace
    EXPECT_EQ(
            FacetId::value_t{FacetId::kAlphabet},
            FacetId::convertIntoValidValue(
                    FacetId::value_t{FacetId::kAlphabet}));
    EXPECT_EQ(
            FacetId::value_t{"+-./0123456789"
                             "@abcdefghijklmnopqrstuvwxyz[]_"
                             "abcdefghijklmnopqrstuvwxyz"},
            FacetId::convertIntoValidValue(
                    "\t!\"#$%&'()*+,-./0123456789:;<=>?"
                    " @ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_"
                    " `abcdefghijklmnopqrstuvwxyz{|}~\n"));
}

TEST_F(FacetIdTest, isValidValue) {
    EXPECT_TRUE(FacetId::isValidValue(QString()));
    EXPECT_TRUE(FacetId::isValidValue(QStringLiteral("facet@mixxx.org/simple-test[1]")));
    EXPECT_FALSE(FacetId::isValidValue(QStringLiteral("Uppercase")));
    EXPECT_FALSE(FacetId::isValidValue(
            QStringLiteral("lower-case_with_digit_1_and_whitespace ")));
    EXPECT_FALSE(FacetId::isValidValue(QLatin1String("")));
    EXPECT_FALSE(FacetId::isValidValue(QStringLiteral(" ")));
}

TEST_F(FacetIdTest, convertedEmptyValueIsValid) {
    EXPECT_TRUE(
            FacetId::isValidValue(FacetId::convertIntoValidValue(FacetId::value_t{})));
    EXPECT_TRUE(
            FacetId::isValidValue(FacetId::convertIntoValidValue("")));
    EXPECT_TRUE(
            FacetId::isValidValue(FacetId::convertIntoValidValue(" ")));
    EXPECT_TRUE(
            FacetId::isValidValue(FacetId::convertIntoValidValue(" \t \n \r ")));
}

class LabelTest : public testing::Test {
};

TEST_F(LabelTest, isEmpty) {
    ASSERT_TRUE(Label().isEmpty());
    // Null string
    EXPECT_EQ(
            Label(),
            Label(Label::value_t()));
    // Empty string
    EXPECT_EQ(
            Label(),
            Label(Label::filterEmptyValue("")));
    // Non-empty string
    EXPECT_FALSE(Label("X").isEmpty());
}

TEST_F(LabelTest, filterEmptyValue) {
    EXPECT_EQ(
            Label::value_t(),
            Label::filterEmptyValue(Label::value_t()));
    EXPECT_EQ(
            Label::value_t(),
            Label::filterEmptyValue(""));
    EXPECT_EQ(
            Label::value_t("X"),
            Label::filterEmptyValue("X"));
}

TEST_F(LabelTest, convertIntoValidValue) {
    // Clamp empty string
    EXPECT_TRUE(
            Label::isValidValue(Label::convertIntoValidValue("")));
    // Clamped whitespace string
    EXPECT_EQ(
            Label::value_t{},
            Label::convertIntoValidValue("  \t\n   "));
    // Trimmed
    EXPECT_EQ(
            Label::value_t{"X y"},
            Label::convertIntoValidValue("  \tX y\n   "));
}

TEST_F(LabelTest, isValidValue) {
    EXPECT_TRUE(Label::isValidValue(QString()));
    EXPECT_TRUE(Label::isValidValue(QStringLiteral("lower-case_with_digit_1")));
    EXPECT_TRUE(Label::isValidValue(QStringLiteral("With\\backslash")));
    EXPECT_TRUE(Label::isValidValue(QStringLiteral("Uppercase with\twhitespace")));
    EXPECT_FALSE(Label::isValidValue(QStringLiteral(" With leading space")));
    EXPECT_FALSE(Label::isValidValue(QStringLiteral("With trailing space\n")));
    EXPECT_FALSE(Label::isValidValue(QLatin1String("")));
    EXPECT_FALSE(Label::isValidValue(QStringLiteral(" ")));
}

TEST_F(LabelTest, convertedEmptyValueIsValid) {
    EXPECT_TRUE(
            Label::isValidValue(Label::convertIntoValidValue(Label::value_t{})));
    EXPECT_TRUE(
            Label::isValidValue(Label::convertIntoValidValue("")));
    EXPECT_TRUE(
            Label::isValidValue(Label::convertIntoValidValue(" ")));
    EXPECT_TRUE(
            Label::isValidValue(FacetId::convertIntoValidValue(" \t \n \r ")));
}

class TagTest : public testing::Test {
};

TEST_F(TagTest, fromJsonValue) {
    const char* labelText = "Label";
    const auto label = Label(labelText);
    const Score::value_t scoreValue = 0.1357;
    const auto score = Score(scoreValue);
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
                    Score::kDefaultValue));
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
    const auto label = Label(labelText);
    const Score::value_t scoreValue = 0.1357;
    const auto score = Score(scoreValue);
    // Only label, no score
    EXPECT_EQ(
            QJsonValue{labelText},
            Tag(label).toJsonValue());
    // Only score, no label
    EXPECT_EQ(
            QJsonValue{Score::kDefaultValue},
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
