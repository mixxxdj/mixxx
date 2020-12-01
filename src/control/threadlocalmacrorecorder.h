#pragma once

#include <QObject>
#include <QThreadStorage>

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
    static QSharedPointer<ThreadLocalMacroRecorder> globalInstance() {
        return s_pMacroRecorder;
    }

    static void setGlobalInstance(QSharedPointer<ThreadLocalMacroRecorder> pMacroRecorder) {
        s_pMacroRecorder = pMacroRecorder;
    }

  private:
    QThreadStorage<MacroRecorder*> m_pMacroRecorder;

    // MacroRecorder instances need this control to exists when they are constructed.
    // We know this is the case since each MacroRecorder instance in QThreadStorage
    // is constructed lazily.
    ControlPushButton m_pCoRecording;

    static QSharedPointer<ThreadLocalMacroRecorder> s_pMacroRecorder;
};
