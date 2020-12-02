#include "quickaction.h"

#include "preferences/usersettings.h"

QuickAction::QuickAction(QObject* parent)
        : QObject(parent),
          m_recordingMacro("[QuickAction]", "recording"),
          m_trigger("[QuickAction]", "trigger"),
          m_recordedValues(),
          m_iBiggestOrdinal(0) {
    m_trigger.connectValueChanged(this, &QuickAction::trigger);
}

bool QuickAction::recordCOValue(const ConfigKey& key, double value) {
    if (m_recordingMacro.toBool()) {
        m_iBiggestOrdinal++;
        m_recordedValues.insert(Key(key, m_iBiggestOrdinal), value);
        return true;
    }
    return false;
}

void QuickAction::trigger(double) {
    auto it = m_recordedValues.constBegin();
    while (it != m_recordedValues.constEnd()) {
        ConfigKey key = it.key().m_configKey;
        double value = it.value();
        ControlProxy(key).set(value);
        it++;
    }
    m_recordedValues.clear();
    m_iBiggestOrdinal = 0;
}

// QMap assumes that two keys x and y are equal if neither x < y nor y < x is true.
bool QuickAction::Key::operator<(const QuickAction::Key& other) const {
    if (m_configKey == other.m_configKey) {
        return false;
    }
    return m_iOrdinal < other.m_iOrdinal;
}
