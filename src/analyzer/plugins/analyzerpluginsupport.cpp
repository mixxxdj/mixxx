#include "analyzer/plugins/analyzerpluginsupport.h"

QString AnalyzerPluginSupportInfo::matchAndSetPluginId(
        const QList<mixxx::AnalyzerPluginInfo>& availablePlugins,
        const QString& pluginIdToMatch) {
    if (const auto plugins = availablePlugins; !plugins.isEmpty()) {
        return std::any_of(std::begin(plugins), std::end(plugins), [&](const auto& info) {
            return info.id() == pluginIdToMatch;
        })
                ? pluginIdToMatch // configured Plug-In available;
                : defaultPlugin(availablePlugins).id();
    }
    return m_pluginId;
}

mixxx::AnalyzerPluginInfo AnalyzerPluginSupportInfo::defaultPlugin(
        const QList<mixxx::AnalyzerPluginInfo>& availablePlugins) const {
    const auto plugins = availablePlugins;
    DEBUG_ASSERT(!plugins.isEmpty());
    return plugins.at(0);
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
