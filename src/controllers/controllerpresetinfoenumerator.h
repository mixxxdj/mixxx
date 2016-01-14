/**
* @file controllerpresetinfoenumerator.h
* @author Be be.0@gmx.com
* @date Sat Jul 18 2015
* @brief Enumerate list of available controller mapping presets
*/
#ifndef CONTROLLERPRESETINFOENUMERATOR_H
#define CONTROLLERPRESETINFOENUMERATOR_H

#include <QMap>
#include <QList>
#include <QString>

#include "controllers/controllerpreset.h"

class PresetInfoEnumerator {
  public:
    PresetInfoEnumerator(const QStringList& searchPaths);
    virtual ~PresetInfoEnumerator();

    bool isValidExtension(const QString extension);

    bool hasPresetInfo(const QString extension, const QString name);
    bool hasPresetInfo(const QString path);

    PresetInfo getPresetInfo(const QString path);

    // Return cached list of presets for this extension
    QList<PresetInfo> getPresets(const QString extension);

    // Updates presets matching given extension
    void updatePresets(const QString extension);

  protected:
    void addExtension(QString extension);
    void loadSupportedPresets();

  private:
    QList<QString> m_fileExtensions;

    // List of paths for controller presets
    QList<QString> m_controllerDirPaths;

    // Cached presets by extension. Map format is:
    // [extension,[preset_path,preset]]
    QMap<QString, QMap<QString, PresetInfo> > m_presetsByExtension;
    QMap<QString, ControllerPresetFileHandler*> m_presetFileHandlersByExtension;
};

#endif
