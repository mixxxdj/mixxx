#pragma once

#include <QMap>
#include <QObject>
#include <QProgressDialog>
#include <memory>

#include "util/duration.h"

namespace mixxx {

typedef double PercentageOfCompletion;

constexpr PercentageOfCompletion kPercentageOfCompletionMin = 0.0;   // not started
constexpr PercentageOfCompletion kPercentageOfCompletionMax = 100.0; // finished

class Task
        : public QObject {
    Q_OBJECT

  public:
    ~Task() override = default;

  protected:
    explicit Task(
            QObject* parent = nullptr)
            : QObject(parent) {
    }

  public slots:
    virtual void slotAbortTask() = 0;
};

class TaskMonitor
        : public QObject {
    Q_OBJECT

  public:
    /// Default minimum duration that needs to pass before showing
    /// the progress dialog.
    ///
    /// For short tasks no progress dialog is shown. Only if their
    /// processing time exceeds the minimum duration the modal
    /// progress dialog is shown. As a side-effect this closes
    /// any open context menu. We want to prevent this distraction
    /// whenever possible while still providing progress feedback
    /// for long running tasks.
    static constexpr Duration kDefaultMinimumProgressDuration =
            Duration::fromMillis(2000);

    explicit TaskMonitor(
            const QString& labelText,
            Duration minimumProgressDuration = kDefaultMinimumProgressDuration,
            QObject* parent = nullptr);
    ~TaskMonitor() override;

    void registerTask(
            Task* pTask,
            const QString& title = QString());
    void unregisterTask(
            Task* pTask);

    void reportTaskProgress(
            Task* pTask,
            PercentageOfCompletion estimatedPercentageOfCompletion,
            const QString& progressMessage = QString());

    void abortAllTasks();

    PercentageOfCompletion sumEstimatedPercentageOfCompletion() const;
    PercentageOfCompletion avgEstimatedPercentageOfCompletion() const;

  public slots:
    void slotRegisterTask(
            const QString& title);
    void slotUnregisterTask();

    void slotReportTaskProgress(
            PercentageOfCompletion estimatedPercentageOfCompletion,
            const QString& progressMessage);

  private:
    Task* senderTask() const;
    void updateProgress();

    const QString m_labelText;
    const Duration m_minimumProgressDuration;

    struct TaskInfo {
        QString title;
        PercentageOfCompletion estimatedPercentageOfCompletion;
        QString progressMessage;
    };
    QMap<Task*, TaskInfo> m_taskInfos;

    std::unique_ptr<QProgressDialog> m_pProgressDlg;
};

} // namespace mixxx
