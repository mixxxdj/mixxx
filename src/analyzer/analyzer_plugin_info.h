#pragma once

#include <QString>

namespace mixxx {

// Plain data class describing an analyzer plugin.
// Extracted from analyzer/plugins/analyzerplugin.h so that preference
// dialogs can hold a QList<AnalyzerPluginInfo> without pulling in the
// full analyzer plugin hierarchy.
class AnalyzerPluginInfo {
  public:
    AnalyzerPluginInfo(const QString& id,
            const QString& author,
            const QString& name,
            bool isConstantTempoSupported)
            : m_id(id),
              m_author(author),
              m_name(name),
              m_isConstantTempoSupported(isConstantTempoSupported) {
    }

    const QString& id() const {
        return m_id;
    }

    const QString& author() const {
        return m_author;
    }

    const QString& name() const {
        return m_name;
    }

    bool isConstantTempoSupported() const {
        return m_isConstantTempoSupported;
    }

  private:
    QString m_id;
    QString m_author;
    QString m_name;
    bool m_isConstantTempoSupported;
};

} // namespace mixxx
