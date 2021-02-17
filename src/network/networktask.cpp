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

void NetworkTask::invokeStart(int timeoutMillis) {
    QMetaObject::invokeMethod(
            this,
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            "slotStart",
            Qt::AutoConnection,
            Q_ARG(int, timeoutMillis)
#else
            [this, timeoutMillis] {
                this->slotStart(timeoutMillis);
            }
#endif
    );
}

void NetworkTask::invokeAbort() {
    QMetaObject::invokeMethod(
            this,
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            "slotAbort"
#else
            [this] {
                this->slotAbort();
            }
#endif
    );
}

void NetworkTask::abort() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    slotAbort();
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
