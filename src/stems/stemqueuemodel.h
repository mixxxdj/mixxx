#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

/// The queue of stem generations shown in the Stems preference page.
///
/// The model owns the jobs: StemGenerator asks it for the next pending one and
/// reports progress back into it, so there is a single place holding state.
class StemQueueModel : public QAbstractListModel {
    Q_OBJECT
  public:
    enum class State {
        Queued,
        Running,
        Finished,
        Failed,
    };
    Q_ENUM(State)

    enum Role {
        TitleRole = Qt::UserRole + 1,
        StatusRole,
        ProgressRole,
        OutputRole,
        ErrorRole,
        StateRole,
        ProviderRole,
    };
    Q_ENUM(Role)

    struct Job {
        QString title;
        QString sourcePath;
        QString outputPath;
        State state{State::Queued};
        int progress{0};
        QString error;
        QString provider;
    };

    explicit StemQueueModel(QObject* pParent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int add(const Job& job);
    bool hasSource(const QString& sourcePath) const;
    /// Row of the first queued job, or -1.
    int nextQueuedRow() const;
    Job jobAt(int row) const;

    void markRunning(int row);
    void setProgress(int row, int progress);
    void markFinished(int row, const QString& provider);
    void markFailed(int row, const QString& error);

    void clearFinished();
    void clearAll();

    int pendingCount() const;

  signals:
    void countsChanged();

  private:
    static QString statusText(const Job& job);

    QList<Job> m_jobs;
};
