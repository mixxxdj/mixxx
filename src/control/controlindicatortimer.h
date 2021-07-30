#pragma once

#include <QObject>
#include <QTimer>
#include <memory>

#include "control/controlobject.h"

namespace mixxx {

/// This class provides two generic indicator controls that change their value
/// in intervals of 250 milliseconds and 500 milliseconds. These controls are
/// used by all `ControlIndicator` COs and may also be used to implement custom
/// blinking buttons (e.g. for Explicit Sync Leader) on controllers. This
/// ensures that all blinking buttons that use the same interval light up at the
/// same time.
class ControlIndicatorTimer : public QObject {
    Q_OBJECT
  public:
    ControlIndicatorTimer(QObject* pParent = nullptr);

  private slots:
    /// Called every 250 milliseconds
    void slotTimeout();

  private:
    QTimer m_timer;
    std::unique_ptr<ControlObject> m_pCOIndicator250millis;
    std::unique_ptr<ControlObject> m_pCOIndicator500millis;
};

} // namespace mixxx
