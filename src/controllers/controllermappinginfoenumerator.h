#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "controllers/controllermappinginfo.h"

/// Enumerate list of available controller mapping mappings
class MappingInfoEnumerator {
  public:
    MappingInfoEnumerator(const QString& searchPath);
    MappingInfoEnumerator(const QStringList& searchPaths);

    // Return cached list of mappings for this extension
    QList<MappingInfo> getMappingsByExtension(const QString& extension);
    void loadSupportedMappings();

  private:
    // List of paths for controller mappings
    QList<QString> m_controllerDirPaths;

    QList<MappingInfo> m_hidMappings;
    QList<MappingInfo> m_midiMappings;
    QList<MappingInfo> m_bulkMappings;
};
