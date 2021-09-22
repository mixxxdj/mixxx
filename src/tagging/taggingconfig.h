#pragma once

#include <QMap>

#include "tagging/migration.h"
#include "tagging/tagmapping.h"

namespace mixxx {

class FacetedTagConfig final {
    MIXXX_DECL_PROPERTY(QString, displayName, DisplayName)
    MIXXX_DECL_PROPERTY(std::optional<TagMappingConfig>, tagMapping, TagMapping)

  public:
    FacetedTagConfig() = default;
    FacetedTagConfig(FacetedTagConfig&&) = default;
    FacetedTagConfig(const FacetedTagConfig&) = default;
    FacetedTagConfig& operator=(FacetedTagConfig&&) = default;
    FacetedTagConfig& operator=(const FacetedTagConfig&) = default;

    explicit FacetedTagConfig(
            const TagMappingConfig& tagMapping)
            : m_tagMapping(tagMapping) {
        DEBUG_ASSERT(isValid());
    }

    static QString clampDisplayName(const QString& displayName) {
        return displayName.trimmed();
    }
    static bool isValidDisplayName(const QString& displayName) {
        return clampDisplayName(displayName) == displayName;
    }

    bool isValid() const {
        return isValidDisplayName(getDisplayName()) &&
                (!getTagMapping() || getTagMapping()->isValid());
    }

    bool hasDisplayName() const {
        return !getDisplayName().isEmpty();
    }

    bool hasTagMapping() const {
        return static_cast<bool>(getTagMapping());
    }

    static std::optional<FacetedTagConfig> fromJsonObject(
            const QJsonObject& jsonObject,
            bool strict = false);
    QJsonObject toJsonObject() const;

  private:
    friend class TaggingConfig;
    QString displayFacet(
            const QString& defaultDisplayName) const {
        DEBUG_ASSERT(isValidDisplayName(defaultDisplayName));
        if (hasDisplayName()) {
            return getDisplayName();
        }
        return defaultDisplayName;
    }
};

bool operator==(
        const FacetedTagConfig& lhs,
        const FacetedTagConfig& rhs);

inline bool operator!=(
        const FacetedTagConfig& lhs,
        const FacetedTagConfig& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const FacetedTagConfig& arg);

typedef QMap<TagFacetId, FacetedTagConfig> FacetedTagConfigMap;

class TaggingConfig final {
    MIXXX_DECL_PROPERTY(Facets, facets, Facets)
    MIXXX_DECL_PROPERTY(FacetedTagConfigMap, customFacets, CustomFacets)

  public:
    TaggingConfig() = default; // empty config
    TaggingConfig(TaggingConfig&&) = default;
    TaggingConfig(const TaggingConfig&) = default;
    TaggingConfig& operator=(TaggingConfig&&) = default;
    TaggingConfig& operator=(const TaggingConfig&) = default;

    explicit TaggingConfig(
            FacetedTagConfigMap&& customFacets)
            : m_customFacets(customFacets) {
    }

    bool isEmpty() const {
        return getCustomFacets().isEmpty() &&
                getFacets().isEmpty();
    }

    void setFacetDisplayName(
            const TagFacetId& facetId,
            const QString& displayName);

    QString displayFacet(
            const TagFacetId& facetId,
            const QString& defaultDisplayName) const {
        if (facetId.isEmpty()) {
            return facetId;
        }
        DEBUG_ASSERT(FacetedTagConfig::isValidDisplayName(defaultDisplayName));
        QString displayName;
        const auto i = getCustomFacets().find(facetId);
        if (i != getCustomFacets().end()) {
            displayName = i->displayFacet(defaultDisplayName);
        } else {
            displayName = defaultDisplayName;
        }
        if (!displayName.isEmpty()) {
            return displayName;
        }
        // Fallback: Return the plain facet identifier
        return facetId;
    }

    /// Find a tag mapping for the given facet.
    ///
    /// The returned pointer is null if no tag mapping has been
    /// found and is only valid while this is unmodified.
    const TagMappingConfig* findFacetTagMapping(
            const TagFacetId& facetId) const;

    TagLabel joinTagsLabel(
            const Facets& facets,
            const TagFacetId& facetId) const {
        const auto* pTagMappingConfig =
                findFacetTagMapping(facetId);
        VERIFY_OR_DEBUG_ASSERT(pTagMappingConfig) {
            return facets.joinTagsLabel(facetId);
        }
        return facets.joinTagsLabel(
                pTagMappingConfig->getLabelSeparator(),
                facetId);
    }

    static std::optional<TaggingConfig> fromJsonObject(
            const QJsonObject& jsonObject,
            bool strict = false);
    QJsonObject toJsonObject() const;

    static std::optional<TaggingConfig> parseJsonData(
            const QByteArray& jsonData,
            bool strict = false);
    QByteArray dumpJsonData() const;

    static std::optional<TaggingConfig> loadFromFile(
            const QString& filePath,
            bool strict = false);
    bool saveIntoFile(
            const QString& filePath) const;
};

bool operator==(
        const TaggingConfig& lhs,
        const TaggingConfig& rhs);

inline bool operator!=(
        const TaggingConfig& lhs,
        const TaggingConfig& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const TaggingConfig& arg);

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::FacetedTagConfig);
Q_DECLARE_METATYPE(mixxx::TaggingConfig);
