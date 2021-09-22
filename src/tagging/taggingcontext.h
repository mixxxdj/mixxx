#pragma once

#include <QMap>
#include <QObject>

#include "tagging/taggingconfig.h"

namespace mixxx {

/// The tagging context for the current session.
class TaggingContext : public QObject {
    Q_OBJECT

    const QMap<TagFacetId, QString> m_defaultFacetDisplayNames;

    const QString m_configFilePath;

    MIXXX_DECL_PROPERTY(TaggingConfig, config, Config)

  public:
    /// Create a new context with an empty(!) configuration
    explicit TaggingContext(
            QObject* parent = nullptr);

    ~TaggingContext() override = default;

    const QString& configFilePath() const {
        return m_configFilePath;
    }

    const FacetedTagConfig* findFacetConfig(
            const TagFacetId& facetId) const;

    QString displayFacet(
            const TagFacetId& facetId) const {
        return getConfig().displayFacet(
                facetId,
                m_defaultFacetDisplayNames.value(facetId));
    }

    /// Create a default configuration with localized (i18n)
    /// facet display names for predefined facets.
    TaggingConfig createDefaultConfig() const;

    void restoreDefaultConfig() {
        refConfig() = createDefaultConfig();
    }

    /// Replace custom configurations for predefined facets
    /// with their corresponding default configurations.
    void restoreDefaultFacetConfigs();

    /// Cleanup and adjust the custom user configuration to match the
    /// current constraints:
    ///
    ///  - remove reserved factes
    ///  - remove custom configurations of predefined facets
    ///
    /// Returns true if the given configuration has been modified
    /// and false otherwise.
    bool sanitizeConfig();

    bool saveConfigFile() const;
    bool reloadConfigFile(bool strict = false);

  private:
    FacetedTagConfigMap createDefaultFacetConfigs() const;

    TaggingConfig loadOrCreateFileConfig(
            const QString& filePath) const;
};

} // namespace mixxx
