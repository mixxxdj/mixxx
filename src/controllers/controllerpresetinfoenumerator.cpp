/**
* @file controllerpresetinfoenumerator.cpp
* @author Be be.0@gmx.com
* @date Sat Jul 18 2015
* @brief Enumerate list of available controller mapping presets
*/
#include "controllers/controllerpresetinfoenumerator.h"

#include <QDirIterator>

#include "controllers/defs_controllers.h"

PresetInfoEnumerator::PresetInfoEnumerator(const QStringList& searchPaths)
        : m_controllerDirPaths(searchPaths) {
    loadSupportedPresets();
}

QList<PresetInfo> PresetInfoEnumerator::getPresetsByExtension(const QString& extension) {
    if (extension == MIDI_PRESET_EXTENSION) {
        return m_midiPresets;
    } else if (extension == HID_PRESET_EXTENSION) {
        return m_hidPresets;
    } else if (extension == BULK_PRESET_EXTENSION) {
        return m_bulkPresets;
    }

    qDebug() << "Extension not registered to presetinfo" << extension;
    return QList<PresetInfo>();
}

void PresetInfoEnumerator::loadSupportedPresets() {
    for (const QString& dirPath : m_controllerDirPaths) {
        QDirIterator it(dirPath);
        while (it.hasNext()) {
            it.next();
            const QString path = it.filePath();

            if (path.endsWith(MIDI_PRESET_EXTENSION, Qt::CaseInsensitive)) {
                m_midiPresets.append(PresetInfo(path));
            } else if (path.endsWith(HID_PRESET_EXTENSION, Qt::CaseInsensitive)) {
                m_hidPresets.append(PresetInfo(path));
            } else if (path.endsWith(BULK_PRESET_EXTENSION, Qt::CaseInsensitive)) {
                m_bulkPresets.append(PresetInfo(path));
            }
        }
    }

    qDebug() << "Extension" << MIDI_PRESET_EXTENSION << "total"
             << m_midiPresets.length() << "presets";
    qDebug() << "Extension" << HID_PRESET_EXTENSION << "total"
             << m_hidPresets.length() << "presets";
    qDebug() << "Extension" << BULK_PRESET_EXTENSION << "total"
             << m_bulkPresets.length() << "presets";
}
