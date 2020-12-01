#pragma once

#include <QObject>

#include "control/controlproxy.h"

class ConfigKey;

class MacroRecorder : public QObject {
    Q_OBJECT
  public:
    explicit MacroRecorder(QObject* parent = nullptr);
    // Record a new value for a CO.
    // Returns true if the macro recorder was in recording state and the new value was recorded,
    // returns false if the new value was not recorded.
    bool recordCOValue(ConfigKey key, double value);

  private:
    ControlProxy m_recordingMacro;
};
