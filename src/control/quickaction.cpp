#include "quickaction.h"

#include "control/controlproxy.h"
#include "preferences/usersettings.h"

QuickAction::QuickAction(int iIndex)
        : QObject(),
          m_coRecording(ConfigKey(QString("[QuickAction%1]").arg(iIndex), "recording")),
          m_coTrigger(ConfigKey(QString("[QuickAction%1]").arg(iIndex), "trigger")),
          m_coClear(ConfigKey(QString("[QuickAction%1]").arg(iIndex), "clear")),
          m_iIndex(iIndex),
          m_recordedValues(100) {
    m_coRecording.setButtonMode(ControlPushButton::TOGGLE);
    m_coTrigger.setButtonMode(ControlPushButton::TRIGGER);
    m_coClear.setButtonMode(ControlPushButton::TRIGGER);

    connect(&m_coTrigger,
            &ControlObject::valueChanged,
            this,
            &QuickAction::slotTriggered);
    connect(&m_coClear,
            &ControlObject::valueChanged,
            this,
            &QuickAction::slotCleared);
}

bool QuickAction::recordCOValue(const ConfigKey& key, double value) {
    if (m_coRecording.toBool()) {
        return m_recordedValues.try_emplace(key, value);
    }
    return false;
}

void QuickAction::trigger() {
    std::vector<std::pair<ConfigKey, double>> values;
    QHash<ConfigKey, unsigned int> validValue;

    m_recordedValues.try_consume_until_current_head(
            [&values, &validValue](QueueElement&& recordedValue) noexcept {
                values.emplace_back(
                        recordedValue.m_key, recordedValue.m_dValue);
                validValue[recordedValue.m_key] = values.size() - 1;
            });

    for (unsigned int i = 0; i < values.size(); ++i) {
        ConfigKey key = values[i].first;
        double value = values[i].second;
        if (validValue[key] == i) {
            ControlProxy(key).set(value);
        }
    }
}

void QuickAction::clear() {
    m_recordedValues.try_consume_until_current_head(
            [](QueueElement&& recordedValue) noexcept {
                Q_UNUSED(recordedValue);
            });
}

void QuickAction::slotTriggered(double d) {
    Q_UNUSED(d);
    m_coRecording.set(0);
    trigger();
}

void QuickAction::slotCleared(double d) {
    Q_UNUSED(d);
    clear();
}
