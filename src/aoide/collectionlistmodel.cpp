#include "aoide/collectionlistmodel.h"

#include <mutex> // std::once_flag

#include "aoide/gateway.h"
#include "aoide/subsystem.h"
#include "util/assert.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("aoide CollectionListModel");

constexpr int kRequestTimeoutMillis = 5000;

std::once_flag registerMetaTypesOnceFlag;

void registerMetaTypesOnce() {
    qRegisterMetaType<aoide::CollectionListFilter>("aoide::CollectionListFilter");
}

} // anonymous namespace

namespace aoide {

CollectionListModel::CollectionListModel(
        Subsystem* subsystem,
        QObject* parent)
        : QAbstractListModel(parent),
          m_subsystem(subsystem) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
    kLogger.debug() << "Created instance" << this;
}

CollectionListModel::~CollectionListModel() {
    kLogger.debug() << "Destroying instance" << this;
    abortPendingTask();
}

int CollectionListModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    DEBUG_ASSERT(!parent.isValid());
    return m_rows.size();
}

QVariant CollectionListModel::data(
        const QModelIndex& index,
        int role) const {
    Q_UNUSED(role)
    DEBUG_ASSERT(!index.parent().isValid());
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return QVariant();
    }
    const auto row = index.row();
    VERIFY_OR_DEBUG_ASSERT(row >= 0 && row < m_rows.size()) {
        return QVariant();
    }
    return QVariant::fromValue(m_rows[row]);
}

void CollectionListModel::abortPendingTask() {
    if (!m_pendingTask) {
        return;
    }
    auto* const pendingTask = m_pendingTask.data();
    pendingTask->disconnect(this);
    pendingTask->invokeAbort();
    pendingTask->deleteLater();
    m_pendingTask.clear();
    DEBUG_ASSERT(!isPending());
    emit pendingChanged(false);
}

void CollectionListModel::onPendingTaskSucceeded(
        const QVector<json::CollectionEntity>& rows) {
    VERIFY_OR_DEBUG_ASSERT(sender() == m_pendingTask.data()) {
        return;
    }
    beginResetModel();
    m_rows = rows;
    endResetModel();
    if (m_filter != m_pendingFilter) {
        m_filter = std::move(m_pendingFilter);
        emit filterChanged(m_filter);
    }
}

void CollectionListModel::onPendingTaskDestroyed(
        QObject* obj) {
    Q_UNUSED(obj)
    if (isPending()) {
        // Another request is already pending
        return;
    }
    emit pendingChanged(false);
}

void CollectionListModel::setFilter(
        const CollectionListFilter& filter) {
    if (isPending()) {
        if (m_pendingFilter == filter) {
            return;
        }
        abortPendingTask();
    }
    DEBUG_ASSERT(!isPending());
    if (m_filter == filter) {
        return;
    }
    loadRows(filter);
}

void CollectionListModel::reload() {
    if (isPending()) {
        return;
    }
    loadRows(m_filter);
}

void CollectionListModel::loadRows(
        const CollectionListFilter& filter) {
    VERIFY_OR_DEBUG_ASSERT(!isPending()) {
        return;
    }
    const auto* subsystem = m_subsystem.data();
    VERIFY_OR_DEBUG_ASSERT(subsystem) {
        return;
    }
    auto* const gateway = subsystem->gateway();
    VERIFY_OR_DEBUG_ASSERT(gateway) {
        // Not connected
        return;
    }
    m_pendingFilter = filter;
    auto* const pendingTask = gateway->listCollections(
            m_pendingFilter.kind);
    DEBUG_ASSERT(pendingTask);
    m_pendingTask = pendingTask;
    connect(pendingTask,
            &ListCollectionsTask::succeeded,
            this,
            &CollectionListModel::onPendingTaskSucceeded,
            Qt::UniqueConnection);
    connect(pendingTask,
            &ListCollectionsTask::destroyed,
            this,
            &CollectionListModel::onPendingTaskDestroyed,
            Qt::UniqueConnection);
    pendingTask->invokeStart(kRequestTimeoutMillis);
    DEBUG_ASSERT(isPending());
    emit pendingChanged(true);
}

} // namespace aoide
