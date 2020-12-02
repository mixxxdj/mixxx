#pragma once

#include <QObject>
#include <utility>

#include "control/controlproxy.h"

class ConfigKey;

class QuickAction : public QObject {
    Q_OBJECT
  public:
    explicit QuickAction(QObject* parent = nullptr);
    // Record a new value for a CO.
    // Returns true if the macro recorder was in recording state and the new value was recorded,
    // returns false if the new value was not recorded.
    bool recordCOValue(const ConfigKey& key, double value);

    void trigger(double value);

  private:
    ControlProxy m_recordingMacro;
    ControlProxy m_trigger;

    // TODO: tests for this
    class Key {
      public:
        Key(ConfigKey configKey, int ordinal)
                : m_configKey(std::move(configKey)), m_iOrdinal(ordinal) {
        }

        // We use configKey to determine whether two keys are equal or not, thus only one Key with a given configKey can
        // exist in the QMap.
        ConfigKey m_configKey;

        // We use ordinal to sort the keys, so the recorded CO are triggered in the same order they were recorded.
        int m_iOrdinal;

        bool operator<(const QuickAction::Key& other) const;
    };
    QMap<Key, double> m_recordedValues;
    int m_iBiggestOrdinal;
};
