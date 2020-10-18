#include "util/taskmonitor.h"

#include <QCoreApplication>
#include <QThread>

#include "util/assert.h"
#include "util/math.h"

namespace mixxx {

TaskMonitor::TaskMonitor(
        const QString& labelText,
        Duration minimumProgressDuration,
        QObject* parent)
        : QObject(parent),
          m_labelText(labelText),
          m_minimumProgressDuration(minimumProgressDuration) {
}

TaskMonitor::~TaskMonitor() {
    VERIFY_OR_DEBUG_ASSERT(m_taskInfos.isEmpty()) {
        // All tasks should have finished now!
        qWarning()
                << "Aborting"
                << m_taskInfos.size()
                << "pending tasks";
        abortAllTasks();
    }
}

Task* TaskMonitor::senderTask() const {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    auto* pTask = static_cast<Task*>(sender());
    DEBUG_ASSERT(pTask);
    DEBUG_ASSERT(dynamic_cast<Task*>(sender()));
    return pTask;
}

void TaskMonitor::slotRegisterTask(
        const QString& title) {
    auto* pTask = senderTask();
    registerTask(pTask, title);
}

void TaskMonitor::slotUnregisterTask() {
    auto* pTask = senderTask();
    unregisterTask(pTask);
}

void TaskMonitor::slotReportTaskProgress(
        PercentageOfCompletion estimatedPercentageOfCompletion,
        const QString& progressMessage) {
    auto* pTask = senderTask();
    reportTaskProgress(
            pTask,
            estimatedPercentageOfCompletion,
            progressMessage);
}

void TaskMonitor::slotCanceled() {
    DEBUG_ASSERT(m_pProgressDlg);
    abortAllTasks();
}

void TaskMonitor::registerTask(
        Task* pTask,
        const QString& title) {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    DEBUG_ASSERT(pTask);
    VERIFY_OR_DEBUG_ASSERT(!m_taskInfos.contains(pTask)) {
        return;
    }
    auto taskInfo = TaskInfo{
            title,
            kPercentageOfCompletionMin,
            QString(),
    };
    m_taskInfos.insert(
            pTask,
            std::move(taskInfo));
    connect(
            pTask,
            &QObject::destroyed,
            this,
            &TaskMonitor::slotUnregisterTask);
    updateProgress();
}

void TaskMonitor::unregisterTask(
        Task* pTask) {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    DEBUG_ASSERT(pTask);
    if (m_taskInfos.remove(pTask) > 0) {
        updateProgress();
    }
}

void TaskMonitor::reportTaskProgress(
        Task* pTask,
        PercentageOfCompletion estimatedPercentageOfCompletion,
        const QString& progressMessage) {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    DEBUG_ASSERT(pTask);
    VERIFY_OR_DEBUG_ASSERT(estimatedPercentageOfCompletion >= kPercentageOfCompletionMin) {
        estimatedPercentageOfCompletion = kPercentageOfCompletionMin;
    }
    VERIFY_OR_DEBUG_ASSERT(estimatedPercentageOfCompletion <= kPercentageOfCompletionMax) {
        estimatedPercentageOfCompletion = kPercentageOfCompletionMax;
    }
    if (estimatedPercentageOfCompletion == kPercentageOfCompletionMax) {
        // Unregister immediately when finished
        unregisterTask(pTask);
        return;
    }
    const auto iTaskInfo = m_taskInfos.find(pTask);
    if (iTaskInfo == m_taskInfos.end()) {
        // Silently ignore (delayed?) progress signals from unregistered tasks
        return;
    }
    iTaskInfo.value().estimatedPercentageOfCompletion = estimatedPercentageOfCompletion;
    iTaskInfo.value().progressMessage = progressMessage;
    updateProgress();
}

void TaskMonitor::abortAllTasks() {
    for (auto* pTask : m_taskInfos.keys()) {
        QMetaObject::invokeMethod(
                pTask,
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
                "slotAbortTask",
                Qt::AutoConnection
#else
                &Task::slotAbortTask
#endif
        );
    }
    m_taskInfos.clear();
    updateProgress();
}

void TaskMonitor::updateProgress() {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    DEBUG_ASSERT(thread() == QCoreApplication::instance()->thread());
    if (m_taskInfos.isEmpty()) {
        m_pProgressDlg.reset();
        return;
    }
    const int currentProgress = static_cast<int>(std::round(sumEstimatedPercentageOfCompletion()));
    if (m_pProgressDlg) {
        m_pProgressDlg->setMaximum(
                static_cast<int>(kPercentageOfCompletionMax * m_taskInfos.size()));
        m_pProgressDlg->setValue(currentProgress);
    } else {
        m_pProgressDlg = std::make_unique<QProgressDialog>(
                m_labelText,
                tr("Abort"),
                currentProgress,
                static_cast<int>(kPercentageOfCompletionMax * m_taskInfos.size()));
        m_pProgressDlg->setWindowModality(Qt::ApplicationModal);
        m_pProgressDlg->setMinimumDuration(m_minimumProgressDuration.toIntegerMillis());
    }
    // TODO: Display the title and optional progress message of each
    // task. Maybe also the individual progress and an option to abort
    // selected tasks.
}

PercentageOfCompletion TaskMonitor::sumEstimatedPercentageOfCompletion() const {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    PercentageOfCompletion sumPercentageOfCompletion = kPercentageOfCompletionMin;
    for (const auto& taskInfo : m_taskInfos) {
        sumPercentageOfCompletion += taskInfo.estimatedPercentageOfCompletion;
    }
    return sumPercentageOfCompletion;
}

PercentageOfCompletion TaskMonitor::avgEstimatedPercentageOfCompletion() const {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    if (m_taskInfos.size() > 0) {
        return sumEstimatedPercentageOfCompletion() / m_taskInfos.size();
    } else {
        return kPercentageOfCompletionMin;
    }
}

} // namespace mixxx
