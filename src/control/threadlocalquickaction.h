#pragma once

#include <QObject>
#include <QThreadStorage>
#include <memory>
#include <utility>

#include "control/controlpushbutton.h"
#include "preferences/configobject.h"

class QuickAction;
class ControlPushButton;

// A class that lazily creates a QuickAction instance for each thread.
//
// Public methods are thread safe
class ThreadLocalQuickAction : public QObject {
    Q_OBJECT
  public:
    explicit ThreadLocalQuickAction(QObject* parent = nullptr);

    // Get the QuickAction instance living in the current thread.
    QuickAction* get();

    QuickAction* operator->() {
        return get();
    }

    // Don't store the shared pointer or the QuickAction CO might leak.
    static std::shared_ptr<ThreadLocalQuickAction> globalInstance() {
        return s_pQuickAction;
    }

    static void setGlobalInstance(std::shared_ptr<ThreadLocalQuickAction> pQuickAction) {
        s_pQuickAction = std::move(pQuickAction);
    }

  private slots:
    void slotTriggered(double);

  private:
    QThreadStorage<QuickAction*> m_pQuickAction;

    // QuickAction instances need these controls to exists when they are constructed.
    // We know this is the case since each QuickAction instance in QThreadStorage
    // is constructed lazily.
    ControlPushButton m_coRecording;
    ControlPushButton m_coTrigger;

    static std::shared_ptr<ThreadLocalQuickAction> s_pQuickAction;
};
