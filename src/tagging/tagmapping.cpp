#include "tagging/tagmapping.h"

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("tagging.mapping");

} // anonymous namespace

TagMappingConfig::TagMappingConfig(
        TagLabel::value_t labelSeparator,
        TagScore::value_t splitScoreAttenuation)
        : m_labelSeparator(std::move(labelSeparator)),
          m_splitScoreAttenuation(splitScoreAttenuation) {
    DEBUG_ASSERT(isValid());
}

//static
std::optional<TagMappingConfig> TagMappingConfig::fromJsonObject(
        const QJsonObject& jsonObject) {
    TagLabel::value_t labelSeparator;
    {
        const auto jsonValue = jsonObject.value(QLatin1String("labelSeparator"));
        if (!jsonValue.isString()) {
            kLogger.warning()
                    << "Invalid JSON value"
                    << jsonValue;
            return std::nullopt;
        }
        labelSeparator = jsonValue.toString();
        if (!isValidLabelSeparator(labelSeparator)) {
            kLogger.warning()
                    << "Invalid label separator"
                    << labelSeparator;
            return std::nullopt;
        }
    }
    TagScore::value_t splitScoreAttenuation;
    {
        const auto jsonValue = jsonObject.value(QLatin1String("splitScoreAttenuation"));
        if (!jsonValue.isDouble()) {
            kLogger.warning()
                    << "Invalid JSON value"
                    << jsonValue;
            return std::nullopt;
        }
        splitScoreAttenuation = jsonValue.toDouble();
        if (!isValidSplitScoreAttenuation(splitScoreAttenuation)) {
            kLogger.warning()
                    << "Invalid split score attenuation"
                    << splitScoreAttenuation;
            return std::nullopt;
        }
    }
    return TagMappingConfig(
            std::move(labelSeparator),
            splitScoreAttenuation);
}

QJsonObject TagMappingConfig::toJsonObject() const {
    return QJsonObject{
            {QLatin1String("labelSeparator"), getLabelSeparator()},
            {QLatin1String("splitScoreAttenuation"), getSplitScoreAttenuation()},
    };
}

bool operator==(
        const TagMappingConfig& lhs,
        const TagMappingConfig& rhs) {
    return lhs.getLabelSeparator() == rhs.getLabelSeparator() &&
            lhs.getSplitScoreAttenuation() == rhs.getSplitScoreAttenuation();
}

QDebug operator<<(
        QDebug dbg,
        const TagMappingConfig& arg) {
    dbg << "TagMappingConfig{";
    arg.dbgLabelSeparator(dbg);
    arg.dbgSplitScoreAttenuation(dbg);
    dbg << '}';
    return dbg;
}

bool TagMappingConfig::labelInputEndsWithSeparator(
        const TagLabel::value_t& labelInput) const {
    const int lastSeparatorIndex =
            labelInput.lastIndexOf(getLabelSeparator());
    if (lastSeparatorIndex == -1) {
        return false;
    }
    DEBUG_ASSERT(lastSeparatorIndex >= 0);
    DEBUG_ASSERT(lastSeparatorIndex < labelInput.size());
    const int rightSize =
            labelInput.size() - (lastSeparatorIndex + getLabelSeparator().size());
    DEBUG_ASSERT(rightSize >= 0);
    if (rightSize == 0) {
        return true;
    }
    return TagLabel::clampValue(labelInput.right(rightSize)).isEmpty();
}

TagLabel::value_t TagMappingConfig::joinLabelsOfOrderedTags(
        const TagVector& tagsOrdered) const {
    QStringList labelValues;
    labelValues.reserve(tagsOrdered.size());
    for (const auto& tag : tagsOrdered) {
        labelValues += tag.getLabel();
    }
    return TagLabel::clampValue(labelValues.join(getLabelSeparator()));
}

TagVector TagMappingConfig::splitLabelIntoOrderedTags(
        const TagLabel::value_t& label,
        TagScore::value_t maxScore) const {
    const auto splitValues = label.split(getLabelSeparator());
    TagVector scoredTags;
    scoredTags.reserve(splitValues.size());
    auto scoreValue = maxScore;
    for (const auto& splitValue : splitValues) {
        const auto labelValue = TagLabel::clampValue(splitValue);
        if (labelValue.isEmpty()) {
            // This might happen while editing an incomplete
            // genre text by appending a separator before
            // adding the next label;
            continue;
        }
        const auto tag = Tag(
                TagLabel(labelValue),
                TagScore(scoreValue));
        scoredTags += tag;
        scoreValue = TagScore::clampValue(
                scoreValue * getSplitScoreAttenuation());
        DEBUG_ASSERT(scoreValue <= tag.getScore());
    }
    return scoredTags;
}

void TagMappingConfig::rescoreOrderedTags(
        TagVector* pOrderedTags,
        TagScore::value_t maxScore) const {
    VERIFY_OR_DEBUG_ASSERT(pOrderedTags) {
        return;
    }
    auto scoreValue = maxScore;
    for (auto& tag : *pOrderedTags) {
        tag.setScore(TagScore(scoreValue));
        scoreValue = TagScore::clampValue(
                scoreValue * getSplitScoreAttenuation());
        DEBUG_ASSERT(scoreValue <= tag.getScore());
    }
}

void TagMappingConfig::rescoreOrderedCustomTags(
        CustomTags* pCustomTags,
        const TagFacet& facet,
        TagScore::value_t maxScore) const {
    VERIFY_OR_DEBUG_ASSERT(pCustomTags) {
        return;
    }
    auto& refFacetedTags = pCustomTags->refFacetedTags(facet);

    auto orderedTags = pCustomTags->getFacetedTagsOrdered(facet);
    rescoreOrderedTags(&orderedTags, maxScore);
    DEBUG_ASSERT(orderedTags.isEmpty() ||
            orderedTags.first().getScore() == maxScore);

    refFacetedTags.clear();
    for (const auto& tag : qAsConst(orderedTags)) {
        refFacetedTags.insert(tag.getLabel(), tag.getScore());
    }
}

bool TagMappingConfig::appendLabelToOrderedCustomTagsAndRescore(
        CustomTags* pCustomTags,
        const TagFacet& facet,
        const TagLabel& newLabel,
        TagScore::value_t maxScore) const {
    VERIFY_OR_DEBUG_ASSERT(pCustomTags) {
        return false;
    }

    auto orderedTags = pCustomTags->getFacetedTagsOrdered(facet);
    if (!orderedTags.isEmpty() &&
            orderedTags.last().getLabel() == newLabel) {
        // Nothing to do (hopefully), i.e. no rescoring needed if none
        // of the ordered tags has been reordered. The score of the primary
        // tag should still match the specified maximum score!
        DEBUG_ASSERT(orderedTags.first().getScore() == maxScore);
        return false;
    }

    // Linear search, but usually just a few items
    for (int i = 0; i < orderedTags.size(); ++i) {
        if (orderedTags[i].getLabel() == newLabel) {
            orderedTags.remove(i);
            break;
        }
    }
    // Actual score doesn't matter
    orderedTags += mixxx::Tag(newLabel);
    rescoreOrderedTags(&orderedTags, maxScore);
    DEBUG_ASSERT(orderedTags.first().getScore() == maxScore);
    DEBUG_ASSERT(orderedTags.last().getLabel() == newLabel);

    auto& refFacetedTags = pCustomTags->refFacetedTags(facet);
    refFacetedTags.clear();
    for (const auto& tag : qAsConst(orderedTags)) {
        refFacetedTags.insert(tag.getLabel(), tag.getScore());
    }
    return true;
}

bool TagMappingConfig::removeLabelFromOrderedCustomTagsAndRescore(
        CustomTags* pCustomTags,
        const TagFacet& facet,
        const TagLabel& oldLabel,
        TagScore::value_t maxScore) const {
    VERIFY_OR_DEBUG_ASSERT(pCustomTags) {
        return false;
    }
    auto& refFacetedTags = pCustomTags->refFacetedTags(facet);

    if (!refFacetedTags.remove(oldLabel)) {
        // Nothing to do (hopefully), i.e. no rescoring needed if none
        // of the ordered tags has been removed. The score of the primary
        // tag should still match the specified maximum score!
        DEBUG_ASSERT(
                pCustomTags->getFacetedTagsOrdered(facet).isEmpty() ||
                pCustomTags->getFacetedTagsOrdered(facet).constFirst().getScore() == maxScore);
        return false;
    }

    auto orderedTags = pCustomTags->getFacetedTagsOrdered(facet);
    rescoreOrderedTags(&orderedTags, maxScore);
    DEBUG_ASSERT(orderedTags.isEmpty() ||
            orderedTags.first().getScore() == maxScore);

    refFacetedTags.clear();
    for (const auto& tag : qAsConst(orderedTags)) {
        refFacetedTags.insert(tag.getLabel(), tag.getScore());
    }
    return true;
}

} // namespace mixxx
