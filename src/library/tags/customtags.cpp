#include "library/tags/customtags.h"

#include <QJsonArray>
#include <QLoggingCategory>

#include "util/json.h"

namespace {

Q_LOGGING_CATEGORY(kLogger, "mixxx.library.tags.CustomTags")

}

namespace mixxx {

namespace library {

namespace tags {

bool CustomTags::isEmpty() const {
    for (auto i = getFacetedTags().begin();
            i != getFacetedTags().end();
            ++i) {
        if (!i.value().isEmpty()) {
            return false;
        }
    }
    return true;
}

void CustomTags::compact() {
    auto i = refFacetedTags().begin();
    while (i != refFacetedTags().end()) {
        if (i.value().isEmpty()) {
            i = refFacetedTags().erase(i);
        } else {
            ++i;
        }
    }
}

bool CustomTags::validate() const {
    for (auto i = getFacetedTags().begin();
            i != getFacetedTags().end();
            ++i) {
        if (i.key().isEmpty()) {
            // plain tags
            for (auto j = i.value().begin();
                    j != i.value().end();
                    ++j) {
                if (j.key().isEmpty() || // label must not be empty
                        !j.key().isValid() ||
                        !j.value().isValid()) {
                    return false;
                }
            }
        } else {
            // faceted tags
            if (!i.key().isValid()) {
                // invalid facetId
                return false;
            }
            for (auto j = i.value().begin();
                    j != i.value().end();
                    ++j) {
                // empty label is allowed
                if (!j.key().isValid() ||
                        !j.value().isValid()) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool CustomTags::containsTag(
        const Label& label,
        const FacetId& facetId) const {
    const auto i = getFacetedTags().find(facetId);
    if (i == getFacetedTags().end()) {
        return false;
    }
    return i.value().contains(label);
}

int CustomTags::countTags(
        const Label& label,
        const FacetId& facetId) const {
    const auto i = getFacetedTags().find(facetId);
    if (i == getFacetedTags().end()) {
        return 0;
    }
    return i.value().count(label);
}

int CustomTags::countFacetedTags(
        const FacetId& facetId) const {
    const auto i = getFacetedTags().find(facetId);
    if (i == getFacetedTags().end()) {
        return 0;
    }
    return i.value().size();
}

bool CustomTags::addOrReplaceTag(
        const Tag& tag,
        const FacetId& facetId) {
    DEBUG_ASSERT(countTags(tag.getLabel(), facetId) <= 1);
    auto i = refFacetedTags().find(facetId);
    if (i == refFacetedTags().end()) {
        i = refFacetedTags().insert(facetId, TagMap{});
    }
    DEBUG_ASSERT(!tag.getLabel().isEmpty() || i.value().size() <= 1);
    DEBUG_ASSERT(i.value().count(tag.getLabel()) <= 1);
    auto j = i.value().find(tag.getLabel());
    if (j == i.value().end()) {
        j = i.value().insert(tag.getLabel(), tag.getScore());
        return true;
    }
    if (j.value() == tag.getScore()) {
        return false;
    }
    j.value() = tag.getScore();
    return true;
}

bool CustomTags::removeTag(
        const Label& label,
        const FacetId& facetId) {
    DEBUG_ASSERT(countTags(label, facetId) <= 1);
    const auto i = refFacetedTags().find(facetId);
    if (i == refFacetedTags().end()) {
        return false;
    }
    DEBUG_ASSERT(!label.isEmpty() || i.value().size() <= 1);
    DEBUG_ASSERT(i.value().count(label) <= 1);
    if (i.value().remove(label) > 0) {
        if (i.value().isEmpty()) {
            // Compact entry for this facetId, i.e. remove it entirely
            refFacetedTags().erase(i);
        }
        return true;
    } else {
        return false;
    }
}

TagVector CustomTags::getTags() const {
    TagVector tags;
    const auto i = getFacetedTags().find(FacetId());
    if (i == getFacetedTags().end()) {
        return tags;
    }
    tags.reserve(i.value().size());
    for (auto j = i.value().begin();
            j != i.value().end();
            ++j) {
        tags += Tag(j.key(), j.value());
    }
    return tags;
}

FacetTagMap::iterator CustomTags::replaceAllFacetedTags(
        const FacetId& facetId,
        const Tag& tag) {
    DEBUG_ASSERT(!facetId.isEmpty());
    DEBUG_ASSERT(countTags(tag.getLabel(), facetId) <= 1);
    auto i = refFacetedTags().find(facetId);
    if (i == refFacetedTags().end()) {
        i = refFacetedTags().insert(facetId, TagMap{});
    } else {
        i.value().clear();
    }
    i.value().insert(tag.getLabel(), tag.getScore());
    DEBUG_ASSERT(countFacetedTags(facetId) == 1);
    return i;
}

int CustomTags::removeAllFacetedTags(
        const FacetId& facetId) {
    DEBUG_ASSERT(!facetId.isEmpty());
    return refFacetedTags().remove(facetId);
}

TagVector CustomTags::getFacetedTagsOrdered(
        const FacetId& facetId) const {
    DEBUG_ASSERT(!facetId.isEmpty());
    TagVector tags;
    auto i = getFacetedTags().find(facetId);
    if (i == getFacetedTags().end()) {
        return tags;
    }
    tags.reserve(i.value().size());
    for (auto j = i.value().begin(); j != i.value().end(); ++j) {
        tags += Tag(j.key(), j.value());
    }
    DEBUG_ASSERT(tags.size() == countFacetedTags(facetId));
    std::sort(
            tags.begin(),
            tags.end(),
            [](const Tag& lhs, const Tag& rhs) {
                return lhs.getScore() > rhs.getScore();
            });
    return tags;
}

Label CustomTags::getFacetedLabel(
        const FacetId& facetId) const {
    DEBUG_ASSERT(countFacetedTags(facetId) <= 1);
    const auto i = getFacetedTags().find(facetId);
    if (i == getFacetedTags().end() ||
            i.value().isEmpty()) {
        return Label();
    }
    DEBUG_ASSERT(i.value().size() == 1);
    DEBUG_ASSERT(i.value().first() == Score());
    return i.value().firstKey();
}

std::optional<Score> CustomTags::getScore(
        const FacetId& facetId,
        const Label& label) const {
    DEBUG_ASSERT(countFacetedTags(facetId) <= 1);
    const auto i = getFacetedTags().find(facetId);
    if (i == getFacetedTags().end() ||
            i.value().isEmpty()) {
        return std::nullopt;
    }
    DEBUG_ASSERT(!label.isEmpty() || i.value().size() <= 1);
    DEBUG_ASSERT(i.value().count(label) <= 1);
    const auto j = i.value().find(label);
    if (j == i.value().end()) {
        return std::nullopt;
    }
    return j.value();
}

Tag CustomTags::mergeFacetedTags(
        const FacetId& facetId,
        AggregateScoring aggregateScoring,
        const QString& joinLabelSeparator) const {
    DEBUG_ASSERT(!facetId.isEmpty());
    const auto tags = getFacetedTagsOrdered(facetId);
    if (tags.isEmpty()) {
        return Tag();
    }
    Score::value_t scoreValue;
    switch (aggregateScoring) {
    case AggregateScoring::Maximum:
        scoreValue = tags.first().getScore();
        break;
    case AggregateScoring::Average:
        scoreValue = 0.0;
        for (const auto& tag : tags) {
            scoreValue += tag.getScore() - Score::kMinValue;
        }
        scoreValue = Score::kMinValue + scoreValue / tags.size();
        DEBUG_ASSERT(Score::isValidValue(scoreValue));
        break;
    default:
        DEBUG_ASSERT(!"unreachable");
        scoreValue = Score();
    }
    QStringList labels;
    labels.reserve(tags.size());
    for (const auto& tag : tags) {
        labels += tag.getLabel();
    }
    auto labelValue =
            Label::convertIntoValidValue(labels.join(joinLabelSeparator));
    return Tag(
            Label(std::move(labelValue)),
            Score(scoreValue));
}

bool CustomTags::addOrReplaceAllTags(
        const CustomTags& tags) {
    bool modified = false;
    VERIFY_OR_DEBUG_ASSERT(this != &tags) {
        return false;
    }
    for (auto i = tags.getFacetedTags().begin();
            i != tags.getFacetedTags().end();
            ++i) {
        for (auto j = i.value().begin();
                j != i.value().end();
                ++j) {
            modified |= addOrReplaceTag(
                    Tag(j.key(), j.value()),
                    i.key());
        }
    }
    return modified;
}

//static
std::optional<CustomTags> CustomTags::fromJsonObject(
        const QJsonObject& jsonObject,
        FromJsonMode mode) {
    CustomTags customTags;
    for (auto i = jsonObject.begin();
            i != jsonObject.end();
            ++i) {
        auto facetIdValue = FacetId::filterEmptyValue(i.key());
        if (!FacetId::isValidValue(facetIdValue)) {
            VERIFY_OR_DEBUG_ASSERT(mode == FromJsonMode::Lenient) {
                qCWarning(kLogger)
                        << "Invalid facetId"
                        << facetIdValue
                        << "from JSON";
                return std::nullopt;
            }
            qCWarning(kLogger)
                    << "Skipping invalid facetId"
                    << facetIdValue
                    << "from JSON";
            continue;
        }
        const auto facetId = FacetId(std::move(facetIdValue));
        DEBUG_ASSERT(i.value().isArray());
        const auto jsonArray = i.value().toArray();
        if (jsonArray.isEmpty()) {
            // Add an empty placeholder slot just for the facetId.
            // This is behavior intended and needed, i.e. when
            // loading a set of predefined tags from a JSON file.
            // Some entries may only contains the name of the facetId,
            // but no predefined tags/labels. The facetId will be
            // displayed in the UI with the option to add custom
            // labels.
            customTags.addOrIgnoreFacet(facetId);
            continue;
        }
        for (const auto& jsonValue : jsonArray) {
            const auto tag = Tag::fromJsonValue(jsonValue);
            if (!tag ||
                    (*tag != Tag() && !tag->isValid())) {
                VERIFY_OR_DEBUG_ASSERT(mode == FromJsonMode::Lenient) {
                    qCWarning(kLogger)
                            << "Invalid tag"
                            << jsonValue
                            << "from JSON for facetId"
                            << facetId.value();
                    return std::nullopt;
                }
                qCWarning(kLogger)
                        << "Skipping invalid tag"
                        << jsonValue
                        << "from JSON for facetId"
                        << facetId.value();
                continue;
            }
            customTags.addOrReplaceTag(
                    std::move(*tag),
                    facetId);
        }
    }
    return customTags;
}

QJsonObject CustomTags::toJsonObject(
        ToJsonMode mode) const {
    QJsonObject jsonObject;
    for (auto i = getFacetedTags().begin();
            i != getFacetedTags().end();
            ++i) {
        QJsonArray jsonArray;
        for (auto j = i.value().begin();
                j != i.value().end();
                ++j) {
            jsonArray += Tag(j.key(), j.value()).toJsonValue();
        }
        if (jsonArray.isEmpty() &&
                mode == ToJsonMode::Compact) {
            // Skip empty plain tags in compact mode
            continue;
        }
        jsonObject.insert(i.key(), jsonArray);
    }
    return jsonObject;
}

//static
std::pair<std::optional<CustomTags>, CustomTags::ParseJsonDataResult> CustomTags::parseJsonData(
        const QByteArray& jsonData,
        FromJsonMode mode) {
    const auto [jsonObject, parseError] = json::parseObject(jsonData);
    if (parseError != QJsonParseError::NoError) {
        qCWarning(kLogger)
                << "Failed to deserialize JSON data"
                << parseError;
        return std::make_pair(std::nullopt, ParseJsonDataResult::DeserializationError);
    }
    auto customTags = CustomTags::fromJsonObject(jsonObject, mode);
    if (!customTags) {
        qCWarning(kLogger)
                << "Failed to transform JSON object"
                << jsonObject;
        return std::make_pair(std::nullopt, ParseJsonDataResult::TransformationError);
    }
    return std::make_pair(customTags, ParseJsonDataResult::Ok);
}

QByteArray CustomTags::dumpJsonData() const {
    return QJsonDocument(toJsonObject()).toJson(QJsonDocument::Compact);
}

bool operator==(
        const CustomTags& lhs,
        const CustomTags& rhs) {
    return lhs.getFacetedTags() == rhs.getFacetedTags();
}

QDebug operator<<(
        QDebug dbg,
        const CustomTags& arg) {
    dbg << "CustomTags{";
    arg.dbgFacetedTags(dbg);
    dbg << '}';
    return dbg;
}

} // namespace tags

} // namespace library

} // namespace mixxx
