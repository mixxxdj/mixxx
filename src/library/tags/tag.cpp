#include "library/tags/tag.h"

#include <QJsonArray>

namespace mixxx {

namespace library {

namespace tags {

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
        auto labelValue = Label::filterEmptyValue(jsonValue.toString());
        if (Label::isValidValue(labelValue)) {
            return Tag(Label(std::move(labelValue)));
        }
    } else if (jsonValue.isDouble()) {
        // score: number
        auto scoreValue = jsonValue.toDouble();
        if (Score::isValidValue(scoreValue)) {
            return Tag(Score(scoreValue));
        }
    } else if (jsonValue.isArray()) {
        // [label: string, score: number]
        auto jsonArray = jsonValue.toArray();
        if (jsonArray.size() == 2 &&
                jsonArray.at(0).isString() &&
                jsonArray.at(1).isDouble()) {
            auto labelValue = Label::filterEmptyValue(jsonArray.at(0).toString());
            auto scoreValue = jsonArray.at(1).toDouble();
            if (Label::isValidValue(labelValue) &&
                    Score::isValidValue(scoreValue)) {
                return Tag(
                        Label(std::move(labelValue)),
                        Score(scoreValue));
            }
        }
    }
    return std::nullopt;
}

QJsonValue Tag::toJsonValue() const {
    if (hasLabel()) {
        // Regular plain tag
        DEBUG_ASSERT(isValid());
        if (getScore() != Score()) {
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
