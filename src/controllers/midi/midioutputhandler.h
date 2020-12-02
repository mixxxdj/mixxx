#pragma once

#include "control/controlproxy.h"
#include "controllers/midi/midimessage.h"

class MidiController;

/// Static MIDI output mapping handler
///
/// This class listens to a control object and sends a midi message based on
/// the  value.
class MidiOutputHandler : public QObject {
    Q_OBJECT
  public:
    MidiOutputHandler(MidiController* controller,
                      const MidiOutputMapping& mapping);
    virtual ~MidiOutputHandler();

    bool validate();
    void update();

  public slots:
    void controlChanged(double value);

  private:
    MidiController* m_pController;
    const MidiOutputMapping m_mapping;
    ControlProxy m_cos;
    int m_lastVal;
};
