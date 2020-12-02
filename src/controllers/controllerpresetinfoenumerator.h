#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "controllers/controllerpresetinfo.h"

/// Enumerate list of available controller mapping presets
class PresetInfoEnumerator {
  public:
    PresetInfoEnumerator(const QString& searchPath);
    PresetInfoEnumerator(const QStringList& searchPaths);

    // Return cached list of presets for this extension
    QList<PresetInfo> getPresetsByExtension(const QString& extension);
    void loadSupportedPresets();

  private:
    // List of paths for controller presets
    QList<QString> m_controllerDirPaths;

    QList<PresetInfo> m_hidPresets;
    QList<PresetInfo> m_midiPresets;
    QList<PresetInfo> m_bulkPresets;
};
