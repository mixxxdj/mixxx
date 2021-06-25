#pragma once

#include <QObject>

#include "control/controlpushbutton.h"
#include "rigtorp/MPMCQueue.h"

class ConfigKey;

class QuickAction : public QObject {
    Q_OBJECT
  public:
    explicit QuickAction(int index);

    // Record a new value for a CO.
    // Returns true if the macro recorder was in recording state and the new value was recorded,
    // returns false if the new value was not recorded.
    bool recordCOValue(const ConfigKey& key, double value);

    void trigger();
    void clear();

  private slots:
    void slotTriggered(double);
    void slotCleared(double);

  private:
    ControlPushButton m_coRecording;
    ControlPushButton m_coTrigger;
    ControlPushButton m_coClear;
    int m_iIndex;

    class QueueElement {
      public:
        QueueElement(ConfigKey key, double dValue) noexcept
                : m_key(key),
                  m_dValue(dValue) {
        }

        ConfigKey m_key;
        double m_dValue;
    };

    rigtorp::MPMCQueue<QueueElement> m_recordedValues;
};
