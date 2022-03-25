#pragma once

#include <QString>

#include "analyzer/analyzer.h"
#include "analyzer/plugins/analyzerplugin.h"

struct AnalyzerPluginSupportInfo {
    virtual QString matchAndSetPluginId(
            const QList<mixxx::AnalyzerPluginInfo>& availablePlugins,
            const QString& pluginIdToMatch);
    virtual mixxx::AnalyzerPluginInfo defaultPlugin(
            const QList<mixxx::AnalyzerPluginInfo>& availablePlugins) const;
    virtual QHash<QString, QString> getExtraVersionInfo(
            const QString& pluginId, bool bPreferencesFastAnalysis) const;

  protected:
    QString m_pluginId;
};
