#include "analyzer/plugins/analyzerpluginsupport.h"

QString AnalyzerPluginSupportInfo::matchOrGetDefaultPluginId(const QString& pluginIdToMatch) const {
    if (const auto plugins = availablePlugins(); !plugins.isEmpty()) {
        if (std::any_of(std::begin(plugins), std::end(plugins), [&](const auto& info) {
                return info.id() == pluginIdToMatch;
            })) {
            return pluginIdToMatch; // configured Plug-In available;
        } else {
            return defaultPlugin();
        }
    }
    return m_pluginId;
}

QString AnalyzerPluginSupportInfo::defaultPlugin() const {
    DEBUG_ASSERT(!availablePlugins().isEmpty());
    return availablePlugins().at(0).id();
}

QHash<QString, QString> AnalyzerPluginSupportInfo::getExtraVersionInfo(
        const QString& pluginId, bool bPreferencesFastAnalysis) const {
    QHash<QString, QString> extraVersionInfo;
    extraVersionInfo["vamp_plugin_id"] = pluginId;
    if (bPreferencesFastAnalysis) {
        extraVersionInfo["fast_analysis"] = "1";
    }
    return extraVersionInfo;
}
