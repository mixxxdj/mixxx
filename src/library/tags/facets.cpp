#include "library/tags/facets.h"

#include <QJsonArray>
#include <QLoggingCategory>

#include "util/json.h"

namespace {

Q_LOGGING_CATEGORY(kLogger, "mixxx.library.tags.Facets")
}

namespace mixxx {

namespace library {

namespace tags {

bool Facets::isEmpty() const {
    for (auto i = m_impl.begin();
            i != m_impl.end();
            ++i) {
        if (!i.value().isEmpty()) {
            return false;
        }
    }
    return true;
}

int Facets::countAllTags() const {
    int count = 0;
    for (auto i = m_impl.begin();
            i != m_impl.end();
            ++i) {
        count += i.value().size();
    }
    return count;
}

void Facets::compact() {
    auto i = m_impl.begin();
    while (i != m_impl.end()) {
        if (i.value().isEmpty()) {
            i = m_impl.erase(i);
        } else {
            ++i;
        }
    }
}

bool Facets::validate() const {
    for (auto i = m_impl.begin();
            i != m_impl.end();
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

bool Facets::containsTagLabeled(
        const Label& label,
        const FacetId& facetId) const {
    const auto i = m_impl.find(facetId);
    if (i == m_impl.end()) {
        return false;
    }
    return i.value().contains(label);
}

int Facets::countTagsLabeled(
        const Label& label,
        const FacetId& facetId) const {
    const auto i = m_impl.find(facetId);
    if (i == m_impl.end()) {
        return 0;
    }
    return i.value().count(label);
}

int Facets::countTags(
        const FacetId& facetId) const {
    const auto i = m_impl.find(facetId);
    if (i == m_impl.end()) {
        return 0;
    }
    return i.value().size();
}

bool Facets::addOrUpdateTag(
        const Tag& tag,
        const FacetId& facetId) {
    DEBUG_ASSERT(countTagsLabeled(tag.getLabel(), facetId) <= 1);
    auto i = m_impl.find(facetId);
    if (i == m_impl.end()) {
        i = m_impl.insert(facetId, TagMap{});
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

bool Facets::removeTagLabeled(
        const Label& label,
        const FacetId& facetId) {
    DEBUG_ASSERT(countTagsLabeled(label, facetId) <= 1);
    const auto i = m_impl.find(facetId);
    if (i == m_impl.end()) {
        return false;
    }
    DEBUG_ASSERT(!label.isEmpty() || i.value().size() <= 1);
    DEBUG_ASSERT(i.value().count(label) <= 1);
    if (i.value().remove(label) > 0) {
        if (i.value().isEmpty()) {
            // Compact entry for this facetId, i.e. remove it entirely
            m_impl.erase(i);
        }
        return true;
    } else {
        return false;
    }
}

TagVector Facets::collectTags(
        const FacetId& facetId) const {
    TagVector tags;
    const auto i = m_impl.find(facetId);
    if (i == m_impl.end()) {
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

TagVector Facets::collectTagsOrdered(
        ScoreOrdering scoreOrdering,
        const FacetId& facetId) const {
    TagVector tags = collectTags(facetId);
    switch (scoreOrdering) {
    case ScoreOrdering::Ascending:
        std::sort(
                tags.begin(),
                tags.end(),
                [](const Tag& lhs, const Tag& rhs) {
                    return lhs.getScore() < rhs.getScore();
                });
        break;
    case ScoreOrdering::Descending:
        std::sort(
                tags.begin(),
                tags.end(),
                [](const Tag& lhs, const Tag& rhs) {
                    return lhs.getScore() > rhs.getScore();
                });
        break;
    }
    return tags;
}

FacetTagMap::iterator Facets::addOrReplaceTagsWithSingleTag(
        const Tag& tag,
        const FacetId& facetId) {
    DEBUG_ASSERT(countTagsLabeled(tag.getLabel(), facetId) <= 1);
    auto i = m_impl.find(facetId);
    if (i == m_impl.end()) {
        i = m_impl.insert(facetId, TagMap{});
    } else {
        i.value().clear();
    }
    i.value().insert(tag.getLabel(), tag.getScore());
    DEBUG_ASSERT(countTags(facetId) == 1);
    return i;
}

int Facets::removeTags(
        const FacetId& facetId) {
    DEBUG_ASSERT(!facetId.isEmpty());
    return m_impl.remove(facetId);
}

std::optional<Tag> Facets::getSingleTag(
        const FacetId& facetId) const {
    DEBUG_ASSERT(countTags(facetId) <= 1);
    const auto i = m_impl.find(facetId);
    if (i == m_impl.end() ||
            i.value().isEmpty()) {
        return std::nullopt;
    }
    DEBUG_ASSERT(i.value().size() <= 1);
    const auto j = i.value().begin();
    if (j == i.value().end()) {
        return std::nullopt;
    }
    return Tag{j.key(), j.value()};
}

std::optional<Score> Facets::getTagScore(
        const Label& label,
        const FacetId& facetId) const {
    DEBUG_ASSERT(countTagsLabeled(label, facetId) <= 1);
    const auto i = m_impl.find(facetId);
    if (i == m_impl.end() ||
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

Tag Facets::mergeTags(
        const QString& joinLabelSeparator,
        AggregateScoring aggregateScoring,
        const FacetId& facetId) const {
    DEBUG_ASSERT(!facetId.isEmpty());
    const auto tags = collectTagsOrdered(ScoreOrdering::Descending, facetId);
    if (tags.isEmpty()) {
        return Tag{};
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

int Facets::addOrUpdateAllTags(
        const Facets& facets) {
    VERIFY_OR_DEBUG_ASSERT(this != &facets) {
        return 0;
    }
    int count = 0;
    for (auto i = facets.m_impl.begin();
            i != facets.m_impl.end();
            ++i) {
        for (auto j = i.value().begin();
                j != i.value().end();
                ++j) {
            if (addOrUpdateTag(
                        Tag{j.key(), j.value()},
                        i.key())) {
                ++count;
            }
        }
    }
    return count;
}

void Facets::addOrReplaceFacet(
        const TagMap& tags,
        const FacetId& facetId) {
    m_impl.insert(facetId, tags);
}

void Facets::addOrReplaceAllFacets(
        const Facets& facets) {
    VERIFY_OR_DEBUG_ASSERT(this != &facets) {
        return;
    }
    for (auto i = facets.m_impl.begin();
            i != facets.m_impl.end();
            ++i) {
        addOrReplaceFacet(i.value(), i.key());
    }
}

//static
std::optional<Facets> Facets::fromJsonObject(
        const QJsonObject& jsonObject,
        FromJsonMode mode) {
    Facets facets;
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
            facets.addOrIgnoreFacet(facetId);
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
            facets.addOrUpdateTag(
                    *tag,
                    facetId);
        }
    }
    return facets;
}

QJsonObject Facets::toJsonObject(
        ToJsonMode mode) const {
    QJsonObject jsonObject;
    for (auto i = m_impl.begin();
            i != m_impl.end();
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
std::pair<std::optional<Facets>, Facets::ParseJsonDataResult> Facets::parseJsonData(
        const QByteArray& jsonData,
        FromJsonMode mode) {
    const auto [jsonObject, parseError] = json::parseObject(jsonData);
    if (parseError != QJsonParseError::NoError) {
        qCWarning(kLogger)
                << "Failed to deserialize JSON data"
                << parseError;
        return std::make_pair(std::nullopt, ParseJsonDataResult::DeserializationError);
    }
    auto facets = Facets::fromJsonObject(jsonObject, mode);
    if (!facets) {
        qCWarning(kLogger)
                << "Failed to transform JSON object"
                << jsonObject;
        return std::make_pair(std::nullopt, ParseJsonDataResult::TransformationError);
    }
    return std::make_pair(facets, ParseJsonDataResult::Ok);
}

QByteArray Facets::dumpJsonData() const {
    return QJsonDocument(toJsonObject()).toJson(QJsonDocument::Compact);
}

bool operator==(
        const Facets& lhs,
        const Facets& rhs) {
    return lhs.m_impl == rhs.m_impl;
}

QDebug operator<<(
        QDebug dbg,
        const Facets& arg) {
    dbg << "Facets{"
        << arg.m_impl
        << '}';
    return dbg;
}

} // namespace tags

} // namespace library

} // namespace mixxx
