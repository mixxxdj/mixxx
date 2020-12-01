#include "macrorecorder.h"

#include "preferences/usersettings.h"

MacroRecorder::MacroRecorder(QObject* parent)
        : QObject(parent),
          m_recordingMacro("[MacroRecorder]", "recording") {
}

bool MacroRecorder::recordCOValue(ConfigKey key, double value) {
    return m_recordingMacro.get();
}
