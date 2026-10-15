#include "stems/stemqueuemodel.h"

#include <QFileInfo>

#include "moc_stemqueuemodel.cpp"

StemQueueModel::StemQueueModel(QObject* pParent)
        : QAbstractListModel(pParent) {
}

int StemQueueModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : m_jobs.size();
}

QString StemQueueModel::statusText(const Job& job) {
    switch (job.state) {
    case State::Queued:
        return tr("Queued");
    case State::Running:
        return tr("Generating %1 %").arg(job.progress);
    case State::Finished:
        return tr("Ready");
    case State::Failed:
        return tr("Failed");
    }
    return QString();
}

QVariant StemQueueModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_jobs.size()) {
        return QVariant();
    }
    const Job& job = m_jobs.at(index.row());
    switch (role) {
    case TitleRole:
        return job.title;
    case StatusRole:
        return statusText(job);
    case ProgressRole:
        return job.state == State::Finished ? 100 : job.progress;
    case OutputRole:
        return job.outputPath;
    case ErrorRole:
        return job.error;
    case StateRole:
        return static_cast<int>(job.state);
    case ProviderRole:
        return job.provider;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> StemQueueModel::roleNames() const {
    return {
            {TitleRole, "title"},
            {StatusRole, "status"},
            {ProgressRole, "progress"},
            {OutputRole, "outputPath"},
            {ErrorRole, "error"},
            {StateRole, "state"},
            {ProviderRole, "provider"},
    };
}

int StemQueueModel::add(const Job& job) {
    const int row = m_jobs.size();
    beginInsertRows(QModelIndex(), row, row);
    m_jobs.append(job);
    endInsertRows();
    emit countsChanged();
    return row;
}

bool StemQueueModel::hasSource(const QString& sourcePath) const {
    for (const Job& job : m_jobs) {
        if (job.sourcePath == sourcePath && job.state != State::Failed) {
            return true;
        }
    }
    return false;
}

int StemQueueModel::nextQueuedRow() const {
    for (int row = 0; row < m_jobs.size(); ++row) {
        if (m_jobs.at(row).state == State::Queued) {
            return row;
        }
    }
    return -1;
}

StemQueueModel::Job StemQueueModel::jobAt(int row) const {
    return (row >= 0 && row < m_jobs.size()) ? m_jobs.at(row) : Job();
}

void StemQueueModel::markRunning(int row) {
    if (row < 0 || row >= m_jobs.size()) {
        return;
    }
    m_jobs[row].state = State::Running;
    m_jobs[row].progress = 0;
    m_jobs[row].error.clear();
    const QModelIndex changed = index(row);
    emit dataChanged(changed, changed);
    emit countsChanged();
}

void StemQueueModel::setProgress(int row, int progress) {
    if (row < 0 || row >= m_jobs.size()) {
        return;
    }
    progress = qBound(0, progress, 100);
    if (m_jobs.at(row).progress == progress) {
        return;
    }
    m_jobs[row].progress = progress;
    const QModelIndex changed = index(row);
    emit dataChanged(changed, changed);
}

void StemQueueModel::markFinished(int row, const QString& provider) {
    if (row < 0 || row >= m_jobs.size()) {
        return;
    }
    m_jobs[row].state = State::Finished;
    m_jobs[row].progress = 100;
    m_jobs[row].provider = provider;
    const QModelIndex changed = index(row);
    emit dataChanged(changed, changed);
    emit countsChanged();
}

void StemQueueModel::markFailed(int row, const QString& error) {
    if (row < 0 || row >= m_jobs.size()) {
        return;
    }
    m_jobs[row].state = State::Failed;
    m_jobs[row].error = error;
    const QModelIndex changed = index(row);
    emit dataChanged(changed, changed);
    emit countsChanged();
}

void StemQueueModel::clearFinished() {
    for (int row = m_jobs.size() - 1; row >= 0; --row) {
        const State state = m_jobs.at(row).state;
        if (state != State::Finished && state != State::Failed) {
            continue;
        }
        beginRemoveRows(QModelIndex(), row, row);
        m_jobs.removeAt(row);
        endRemoveRows();
    }
    emit countsChanged();
}

void StemQueueModel::clearAll() {
    if (m_jobs.isEmpty()) {
        return;
    }
    beginResetModel();
    m_jobs.clear();
    endResetModel();
    emit countsChanged();
}

int StemQueueModel::pendingCount() const {
    int count = 0;
    for (const Job& job : m_jobs) {
        if (job.state == State::Queued || job.state == State::Running) {
            ++count;
        }
    }
    return count;
}
