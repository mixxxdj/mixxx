/**
 * @file midicontrollerpresetfilehandler.h
 * @author Sean Pappalardo spappalardo@mixxx.org
 * @date Mon 9 Apr 2012
 * @brief Handles loading and saving of MIDI controller presets.
 */

#ifndef MIDICONTROLLERPRESETFILEHANDLER_H
#define MIDICONTROLLERPRESETFILEHANDLER_H

#include "controllers/controllerpresetfilehandler.h"
#include "controllers/midi/midicontrollerpreset.h"

class MidiControllerPresetFileHandler : public ControllerPresetFileHandler {
  public:
    MidiControllerPresetFileHandler() {};
    virtual ~MidiControllerPresetFileHandler() {};

    bool save(const MidiControllerPreset& preset, const QString& fileName) const;

  private:
    virtual ControllerPresetPointer load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemPresetPath);

    void addControlsToDocument(const MidiControllerPreset& preset,
                               QDomDocument* doc) const;

    QDomElement makeTextElement(QDomDocument* doc,
                                const QString& elementName,
                                const QString& text) const;

    QDomElement inputMappingToXML(QDomDocument* doc,
                                  const MidiInputMapping& mapping) const;

    QDomElement outputMappingToXML(QDomDocument* doc,
                                   const MidiOutputMapping& mapping) const;
};

#endif
