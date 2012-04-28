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
#include "controllers/mixxxcontrol.h"

class MidiControllerPresetFileHandler : public ControllerPresetFileHandler {
  public:
    MidiControllerPresetFileHandler() {};
    virtual ~MidiControllerPresetFileHandler() {};

    bool save(const MidiControllerPreset& preset,
              const QString deviceName, const QString fileName) const;

  private:
    virtual ControllerPresetPointer load(const QDomElement root, const QString deviceName,
                                         const bool forceLoad);

    void addControlsToDocument(const MidiControllerPreset& preset,
                               QDomDocument* doc) const;

    void mappingToXML(QDomElement& parentNode, MixxxControl mc,
                      unsigned char status, unsigned char control) const;

    void outputMappingToXML(QDomElement& parentNode, unsigned char on,
                            unsigned char off, float max, float min) const;
};

#endif
