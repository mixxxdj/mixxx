#pragma once

#include <QJsonObject>

#include "tagging/customtags.h"

namespace mixxx {

class TagMappingConfig final {
    MIXXX_DECL_PROPERTY(TagLabel::value_t, labelSeparator, LabelSeparator)
    MIXXX_DECL_PROPERTY(TagScore::value_t, splitScoreAttenuation, SplitScoreAttenuation)

  public:
    static bool isValidLabelSeparator(
            const TagLabel::value_t& labelSeparator) {
        return !labelSeparator.isEmpty();
    }
    static bool isValidSplitScoreAttenuation(
            TagScore::value_t splitScoreAttenuation) {
        return splitScoreAttenuation >= 0 &&
                splitScoreAttenuation <= 1;
    }

    TagMappingConfig()
            : m_splitScoreAttenuation(TagScore::kDefaultValue) {
    }
    TagMappingConfig(
            TagLabel::value_t labelSeparator,
            TagScore::value_t splitScoreAttenuation);
    TagMappingConfig(TagMappingConfig&&) = default;
    TagMappingConfig(const TagMappingConfig&) = default;
    TagMappingConfig& operator=(TagMappingConfig&&) = default;
    TagMappingConfig& operator=(const TagMappingConfig&) = default;

    bool hasValidLabelSeparator() const {
        return isValidLabelSeparator(getLabelSeparator());
    }
    bool hasValidSplitScoreAttenuation() const {
        return isValidSplitScoreAttenuation(getSplitScoreAttenuation());
    }

    bool isValid() const {
        return hasValidLabelSeparator() &&
                hasValidSplitScoreAttenuation();
    }

    bool labelInputEndsWithSeparator(
            const TagLabel::value_t& labelInput) const;

    TagLabel::value_t joinLabelsOfOrderedTags(
            const TagVector& tagsOrdered) const;
    TagVector splitLabelIntoOrderedTags(
            const TagLabel::value_t& label,
            TagScore::value_t maxScore = TagScore::kMaxValue) const;

    void rescoreOrderedTags(
            TagVector* pOrderedTags,
            TagScore::value_t maxScore = TagScore::kMaxValue) const;

    void rescoreOrderedCustomTags(
            CustomTags* pCustomTags,
            const TagFacet& facet,
            TagScore::value_t maxScore = TagScore::kMaxValue) const;
    bool appendLabelToOrderedCustomTagsAndRescore(
            CustomTags* pCustomTags,
            const TagFacet& facet,
            const TagLabel& newLabel,
            TagScore::value_t maxScore = TagScore::kMaxValue) const;
    bool removeLabelFromOrderedCustomTagsAndRescore(
            CustomTags* pCustomTags,
            const TagFacet& facet,
            const TagLabel& oldLabel,
            TagScore::value_t maxScore = TagScore::kMaxValue) const;

    static std::optional<TagMappingConfig> fromJsonObject(
            const QJsonObject& jsonObject);
    QJsonObject toJsonObject() const;
};

bool operator==(
        const TagMappingConfig& lhs,
        const TagMappingConfig& rhs);

inline bool operator!=(
        const TagMappingConfig& lhs,
        const TagMappingConfig& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const TagMappingConfig& arg);

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::TagMappingConfig);
