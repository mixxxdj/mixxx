#include "quickaction.h"

#include "preferences/usersettings.h"

QuickAction::QuickAction(QObject* parent)
        : QObject(parent),
          m_recordingMacro("[QuickAction]", "recording"),
          m_trigger("[QuickAction]", "trigger") {
    m_trigger.connectValueChanged(this, &QuickAction::trigger);
}

bool QuickAction::recordCOValue(const ConfigKey& key, double value) {
    if (m_recordingMacro.toBool()) {
        m_recordedValues.emplace_back(key, value);
        m_validPosition[key] = m_recordedValues.size() - 1;
        return true;
    }
    return false;
}

void QuickAction::trigger(double) {
    for (unsigned int i = 0; i < m_recordedValues.size(); ++i) {
        ConfigKey key = m_recordedValues[i].first;
        double value = m_recordedValues[i].second;
        if (m_validPosition[key] == i) {
            ControlProxy(key).set(value);
        }
    }
    m_recordedValues.clear();
    m_validPosition.clear();
}
