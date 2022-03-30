#pragma once

#include <QString>

#include "analyzer/analyzer.h"
#include "analyzer/plugins/analyzerplugin.h"

class AnalyzerPluginSupportInfo {
  public:
    virtual QList<mixxx::AnalyzerPluginInfo> availablePlugins() const = 0;
    QString matchAndSetPluginId(const QString& pluginIdToMatch);
    QString defaultPlugin() const;
    virtual QHash<QString, QString> getExtraVersionInfo(
            const QString& pluginId, bool bPreferencesFastAnalysis) const;

  protected:
    QString m_pluginId;
};
