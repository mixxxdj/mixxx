#include "network/networktask.h"

#include "moc_networktask.cpp"
#include "util/counter.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace mixxx {

namespace network {

namespace {

const Logger kLogger("mixxx::network::NetworkTask");

// count = even number (ctor + dtor)
// sum = 0 (no memory leaks)
Counter s_instanceCounter(QStringLiteral("mixxx::network::NetworkTask"));

} // anonymous namespace

NetworkTask::NetworkTask(
        QNetworkAccessManager* networkAccessManager,
        QObject* parent)
        : QObject(parent),
          m_networkAccessManagerWeakPtr(networkAccessManager) {
    s_instanceCounter.increment(1);
}

NetworkTask::~NetworkTask() {
    s_instanceCounter.increment(-1);
}

void NetworkTask::invokeStart(int timeoutMillis, int delayMillis) {
    QMetaObject::invokeMethod(
            this,
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            "slotStart",
            Qt::AutoConnection,
            Q_ARG(int, timeoutMillis),
            Q_ARG(int, delayMillis)
#else
            [this, timeoutMillis, delayMillis] {
                this->slotStart(timeoutMillis, delayMillis);
            }
#endif
    );
}

void NetworkTask::invokeAbort() {
    QMetaObject::invokeMethod(
            this,
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            "slotAbort",
            Q_ARG(bool, false)
#else
            [this] {
                this->slotAbort(false);
            }
#endif
    );
}

void NetworkTask::abort() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    slotAbort(false);
}

void NetworkTask::emitAborted(
        const QUrl& requestUrl) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&NetworkTask::aborted)) {
        kLogger.warning()
                << this
                << "Unhandled abort signal"
                << requestUrl;
        deleteLater();
        return;
    }
    emit aborted(requestUrl);
}

} // namespace network

} // namespace mixxx
