#pragma once

#include <QObject>
#include <QThreadStorage>
#include <memory>
#include <utility>

#include "control/controlpushbutton.h"
#include "preferences/configobject.h"

class MacroRecorder;
class ControlPushButton;

// A class that lazily creates a MacroRecorder instance for each thread.
//
// Public methods are thread safe
class ThreadLocalMacroRecorder : public QObject {
    Q_OBJECT
  public:
    explicit ThreadLocalMacroRecorder(QObject* parent = nullptr);

    // Get the MacroRecorder instance living in the current thread.
    MacroRecorder* get();

    MacroRecorder* operator->() {
        return get();
    }

    // Don't store the shared pointer or the MacroRecorder CO might leak.
    static std::shared_ptr<ThreadLocalMacroRecorder> globalInstance() {
        return s_pMacroRecorder;
    }

    static void setGlobalInstance(std::shared_ptr<ThreadLocalMacroRecorder> pMacroRecorder) {
        s_pMacroRecorder = std::move(pMacroRecorder);
    }

  private:
    void slotTriggered(double);

    QThreadStorage<MacroRecorder*> m_pMacroRecorder;

    // MacroRecorder instances need these controls to exists when they are constructed.
    // We know this is the case since each MacroRecorder instance in QThreadStorage
    // is constructed lazily.
    ControlPushButton m_coRecording;
    ControlPushButton m_coTrigger;

    static std::shared_ptr<ThreadLocalMacroRecorder> s_pMacroRecorder;
};
