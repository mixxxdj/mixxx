#include "macrorecorder.h"

#include "preferences/usersettings.h"

MacroRecorder::MacroRecorder(QObject* parent)
        : QObject(parent),
          m_recordingMacro("[MacroRecorder]", "recording"),
          m_trigger("[MacroRecorder]", "trigger"),
          m_recordedValues(),
          m_iBiggestOrdinal(0) {
    m_trigger.connectValueChanged(this, &MacroRecorder::trigger);
}

bool MacroRecorder::recordCOValue(const ConfigKey& key, double value) {
    if (m_recordingMacro.toBool()) {
        m_iBiggestOrdinal++;
        m_recordedValues.insert(Key(key, m_iBiggestOrdinal), value);
        return true;
    }
    return false;
}

void MacroRecorder::trigger(double) {
    auto it = m_recordedValues.constKeyValueBegin();
    while (it != m_recordedValues.constKeyValueEnd()) {
        ConfigKey key = (*it).first.m_configKey;
        double value = (*it).second;
        ControlProxy(key).set(value);
        it++;
    }
    m_recordedValues.clear();
    m_iBiggestOrdinal = 0;
}

// QMap assumes that two keys x and y are equal if neither x < y nor y < x is true.
bool MacroRecorder::Key::operator<(const MacroRecorder::Key& other) const {
    if (m_configKey == other.m_configKey) {
        return false;
    }
    return m_iOrdinal < other.m_iOrdinal;
}
