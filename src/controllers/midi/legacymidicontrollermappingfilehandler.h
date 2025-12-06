#pragma once

#include <gtest/gtest_prod.h>

#include "controllers/legacycontrollermappingfilehandler.h"
#include "controllers/midi/legacymidicontrollermapping.h"

/// Handles loading and saving of MIDI controller mappings.
class LegacyMidiControllerMappingFileHandler : public LegacyControllerMappingFileHandler {
  public:
    LegacyMidiControllerMappingFileHandler(){};
    virtual ~LegacyMidiControllerMappingFileHandler(){};

    bool save(const LegacyMidiControllerMapping& mapping, const QString& fileName) const;

  private:
    virtual std::shared_ptr<LegacyControllerMapping> load(const QDomElement& root,
            const QString& filePath,
            const QDir& systemMappingPath);

    void addControlsToDocument(const LegacyMidiControllerMapping& mapping,
            QDomDocument* doc) const;

    QDomElement makeTextElement(QDomDocument* doc,
            const QString& elementName,
            const QString& text) const;

    QDomElement inputMappingToXML(QDomDocument* doc,
            const MidiInputMapping& mapping) const;

    QDomElement outputMappingToXML(QDomDocument* doc,
            const MidiOutputMapping& mapping) const;

    FRIEND_TEST(LegacyControllerMappingFileHandlerTest, canSerializeMappingToFile);
};
