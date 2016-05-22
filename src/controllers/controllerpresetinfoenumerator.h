/**
* @file controllerpresetinfoenumerator.h
* @author Be be.0@gmx.com
* @date Sat Jul 18 2015
* @brief Enumerate list of available controller mapping presets
*/
#ifndef CONTROLLERPRESETINFOENUMERATOR_H
#define CONTROLLERPRESETINFOENUMERATOR_H

#include <QList>
#include <QString>
#include <QStringList>

#include "controllers/controllerpresetinfo.h"

class PresetInfoEnumerator {
  public:
    PresetInfoEnumerator(const QStringList& searchPaths);

    // Return cached list of presets for this extension
    QList<PresetInfo> getPresetsByExtension(const QString& extension);

  protected:
    void loadSupportedPresets();

  private:
    // List of paths for controller presets
    QList<QString> m_controllerDirPaths;

    QList<PresetInfo> m_hidPresets;
    QList<PresetInfo> m_midiPresets;
    QList<PresetInfo> m_bulkPresets;
};

#endif
