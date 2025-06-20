#include "util/task.h"

#include <QtDebug>

#include "moc_task.cpp"

TaskWatcher::~TaskWatcher() {
    if (m_activeTasks.loadRelaxed() > 0) {
        qWarning() << "TaskWatcher destroyed before all tasks were done.";
    }
}

void TaskWatcher::watchTask() {
    // Increment the number of active tasks.
    m_activeTasks.ref();
}

void TaskWatcher::taskDone() {
    // Decrement m_activeTasks and if it is zero emit allTasksDone().
    if (!m_activeTasks.deref()) {
        emit allTasksDone();
    }
}
