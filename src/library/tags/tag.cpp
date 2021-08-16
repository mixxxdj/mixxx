#include "library/tags/tag.h"

#include <QJsonArray>
#include <QRegularExpression>

namespace {

const QRegularExpression kLowercaseAsciiNotEmpty(
        QStringLiteral("^[\\x{0021}-\\x{0040}\\x{005B}-\\x{007E}]+"));
const QRegularExpression kInverseLowercaseAsciiNotEmpty(
        QStringLiteral("[^\\x{0021}-\\x{0040}\\x{005B}-\\x{007E}]+"));

} // anonymous namespace

namespace mixxx {

namespace library {

namespace tags {

//static
bool TagFacet::isValidValue(
        const value_t& value) {
    if (value.isNull()) {
        return true;
    }
    if (value.isEmpty()) {
        // for disambiguation with null
        return false;
    }
    const auto match = kLowercaseAsciiNotEmpty.match(value);
    DEBUG_ASSERT(match.isValid());
    DEBUG_ASSERT(value.length() > 0);
    // match = exact match
    return match.capturedLength() == value.length();
}

//static
TagFacet::value_t TagFacet::clampValue(
        const value_t& value) {
    auto clampedValue = filterEmptyValue(value.toLower().remove(kInverseLowercaseAsciiNotEmpty));
    DEBUG_ASSERT(isValidValue(clampedValue));
    return clampedValue;
}

//static
bool TagLabel::isValidValue(
        const value_t& value) {
    if (value.isNull()) {
        return true;
    }
    if (value.isEmpty()) {
        // for disambiguation with null
        return false;
    }
    return value.trimmed() == value;
}

//static
TagLabel::value_t TagLabel::clampValue(
        const value_t& value) {
    auto clampedValue = filterEmptyValue(value.trimmed());
    DEBUG_ASSERT(isValidValue(clampedValue));
    return clampedValue;
}

bool operator==(
        const Tag& lhs,
        const Tag& rhs) {
    return lhs.getLabel() == rhs.getLabel() &&
            lhs.getScore() == rhs.getScore();
}

QDebug operator<<(
        QDebug dbg,
        const Tag& arg) {
    dbg << "Tag{";
    arg.dbgLabel(dbg);
    arg.dbgScore(dbg);
    dbg << '}';
    return dbg;
}

std::optional<Tag> Tag::fromJsonValue(
        const QJsonValue& jsonValue) {
    if (jsonValue.isString()) {
        // label: string
        auto labelValue = TagLabel::filterEmptyValue(jsonValue.toString());
        if (TagLabel::isValidValue(labelValue)) {
            return Tag(TagLabel(std::move(labelValue)));
        }
    } else if (jsonValue.isDouble()) {
        // score: number
        auto scoreValue = jsonValue.toDouble();
        if (TagScore::isValidValue(scoreValue)) {
            return Tag(TagScore(scoreValue));
        }
    } else if (jsonValue.isArray()) {
        // [label: string, score: number]
        auto jsonArray = jsonValue.toArray();
        if (jsonArray.size() == 2 &&
                jsonArray.at(0).isString() &&
                jsonArray.at(1).isDouble()) {
            auto labelValue = TagLabel::filterEmptyValue(jsonArray.at(0).toString());
            auto scoreValue = jsonArray.at(1).toDouble();
            if (TagLabel::isValidValue(labelValue) &&
                    TagScore::isValidValue(scoreValue)) {
                return Tag(
                        TagLabel(std::move(labelValue)),
                        TagScore(scoreValue));
            }
        }
    }
    return std::nullopt;
}

QJsonValue Tag::toJsonValue() const {
    if (hasLabel()) {
        // Regular plain tag
        DEBUG_ASSERT(isValid());
        if (getScore() != TagScore()) {
            return QJsonArray{
                    QJsonValue{getLabel()},
                    QJsonValue{getScore()}};
        } else {
            // Omit the default score
            return QJsonValue{getLabel()};
        }
    } else {
        // Empty label, may only happen when part of a faceted tag
        DEBUG_ASSERT(getLabel().isEmpty());
        return QJsonValue{getScore()};
    }
}

} // namespace tags

} // namespace library

} // namespace mixxx
