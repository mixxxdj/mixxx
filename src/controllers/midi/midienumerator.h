#pragma once

#include "controllers/controllerenumerator.h"

/// @brief Base class handling discovery and enumeration of DJ controllers that
/// use the MIDI protocol.
///
/// This class handles discovery and enumeration of MIDI DJ controllers and
///   must be inherited by a class that implements it on some API.
class MidiEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    MidiEnumerator();
    virtual ~MidiEnumerator();

    virtual QList<Controller*> queryDevices() = 0;
};
