#include <QtDebug>

#include "util/compatibility.h"
#include "util/task.h"

TaskWatcher::TaskWatcher(QObject* pParent) : QObject(pParent) {
}

TaskWatcher::~TaskWatcher() {
    if (load_atomic(m_activeTasks) > 0) {
        qWarning() << "TaskWatcher destroyed before all tasks were done.";
    }
}

void TaskWatcher::watchTask(QObject* pTask, const char* doneSignal) {
    // Increment the number of active tasks.
    m_activeTasks.ref();

    // Watch pTask for doneSignal. Use a DirectConnection since m_activeTasks is
    // an atomic integer.
    connect(pTask, doneSignal, this, SLOT(taskDone()), Qt::DirectConnection);
}

void TaskWatcher::taskDone() {
    // Decrement m_activeTasks and if it is zero emit allTasksDone().
    if (!m_activeTasks.deref()) {
        emit(allTasksDone());
    }
}
