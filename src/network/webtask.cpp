#include "network/webtask.h"

#include <QTimerEvent>
#include <mutex> // std::once_flag

#include "util/assert.h"
#include "util/counter.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace mixxx {

namespace network {

namespace {

const Logger kLogger("mixxx::network::WebTask");

constexpr int kInvalidTimerId = -1;

// count = even number (ctor + dtor)
// sum = 0 (no memory leaks)
Counter s_instanceCounter(QStringLiteral("mixxx::network::WebTask"));

std::once_flag registerMetaTypesOnceFlag;

void registerMetaTypesOnce() {
    WebResponse::registerMetaType();
    CustomWebResponse::registerMetaType();
}

bool readStatusCode(
        const QNetworkReply* reply,
        int* statusCode) {
    DEBUG_ASSERT(statusCode);
    const QVariant statusCodeAttr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    bool statusCodeValid = false;
    const int statusCodeValue = statusCodeAttr.toInt(&statusCodeValid);
    VERIFY_OR_DEBUG_ASSERT(statusCodeValid && HttpStatusCode_isValid(statusCodeValue)) {
        kLogger.warning()
                << "Invalid or missing status code attribute"
                << statusCodeAttr;
    }
    else {
        *statusCode = statusCodeValue;
    }
    return statusCodeValid;
}

} // anonymous namespace

/*static*/ void WebResponse::registerMetaType() {
    qRegisterMetaType<WebResponse>("mixxx::network::WebResponse");
}

QDebug operator<<(QDebug dbg, const WebResponse& arg) {
    return dbg
        << "WebResponse{"
        << arg.replyUrl
        << arg.statusCode
        << '}';
}

/*static*/ void CustomWebResponse::registerMetaType() {
    qRegisterMetaType<CustomWebResponse>("mixxx::network::CustomWebResponse");
}

QDebug operator<<(QDebug dbg, const CustomWebResponse& arg) {
    return dbg
        << "CustomWebResponse{"
        << static_cast<const WebResponse&>(arg)
        << arg.content
        << '}';
}

WebTask::WebTask(
        QNetworkAccessManager* networkAccessManager,
        QObject* parent)
        : QObject(parent),
          m_networkAccessManager(networkAccessManager),
          m_timeoutTimerId(kInvalidTimerId),
          m_status(Status::Idle) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
    DEBUG_ASSERT(m_networkAccessManager);
    s_instanceCounter.increment(1);
}

WebTask::~WebTask() {
    s_instanceCounter.increment(-1);
}

void WebTask::onAborted(
        QUrl&& requestUrl) {
    DEBUG_ASSERT(m_status == Status::Aborted);
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&WebTask::aborted)) {
        kLogger.warning()
                << "Unhandled abort signal"
                << requestUrl;
        deleteLater();
        return;
    }
    emit aborted(
            std::move(requestUrl));
}

void WebTask::onTimedOut(
        QUrl&& requestUrl) {
    DEBUG_ASSERT(m_status == Status::TimedOut);
    onNetworkError(
            std::move(requestUrl),
            QNetworkReply::TimeoutError,
            tr("Client-side network timeout"),
            QByteArray());
}

void WebTask::onNetworkError(
        QUrl&& requestUrl,
        QNetworkReply::NetworkError errorCode,
        QString&& errorString,
        QByteArray&& errorContent) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&WebTask::networkError)) {
        kLogger.warning()
                << "Unhandled network error signal"
                << requestUrl
                << errorCode
                << errorString
                << errorContent;
        deleteLater();
        return;
    }
    emit networkError(
            std::move(requestUrl),
            errorCode,
            std::move(errorString),
            std::move(errorContent));
}

void WebTask::invokeStart(int timeoutMillis) {
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

void WebTask::invokeAbort() {
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

void WebTask::slotStart(int timeoutMillis) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(m_status != Status::Pending);
    VERIFY_OR_DEBUG_ASSERT(m_networkAccessManager) {
        onNetworkError(
                QUrl(),
                QNetworkReply::NetworkSessionFailedError,
                tr("No network access"),
                QByteArray());
        return;
    }

    kLogger.debug()
            << "Starting...";
    m_status = Status::Idle;
    if (!doStart(m_networkAccessManager, timeoutMillis)) {
        // Still idle, because we are in the same thread.
        // The callee is not supposed to abort a request
        // before it has beeen started successfully.
        DEBUG_ASSERT(m_status == Status::Idle);
        onNetworkError(
                QUrl(),
                QNetworkReply::OperationCanceledError,
                tr("Start of network task has been aborted"),
                QByteArray());
        return;
    }
    // Still idle after the request has been started
    // successfully, i.e. nothing happened yet in this
    // thread.
    DEBUG_ASSERT(m_status == Status::Idle);
    m_status = Status::Pending;

    DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
    if (timeoutMillis > 0) {
        m_timeoutTimerId = startTimer(timeoutMillis);
        DEBUG_ASSERT(m_timeoutTimerId != kInvalidTimerId);
    }
}

QUrl WebTask::abortPendingNetworkReply(
        QNetworkReply* pendingNetworkReply) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(pendingNetworkReply);
    if (pendingNetworkReply->isRunning()) {
        pendingNetworkReply->abort();
        // Suspend until finished
        return QUrl();
    }
    return pendingNetworkReply->request().url();
}

QUrl WebTask::timeOutPendingNetworkReply(
        QNetworkReply* pendingNetworkReply) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(pendingNetworkReply);
    if (pendingNetworkReply->isRunning()) {
        //pendingNetworkReply->abort();
        // Don't suspend until finished, i.e. abort and then
        // delete the pending network request instantly
    }
    return pendingNetworkReply->request().url();
}

QUrl WebTask::abort() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_status != Status::Pending) {
        DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
        return QUrl();
    }
    if (m_timeoutTimerId != kInvalidTimerId) {
        killTimer(m_timeoutTimerId);
        m_timeoutTimerId = kInvalidTimerId;
    }
    m_status = Status::Aborted;
    kLogger.debug()
            << "Aborting...";
    QUrl url = doAbort();
    onAborted(QUrl(url));
    return url;
}

void WebTask::slotAbort() {
    abort();
}

void WebTask::timerEvent(QTimerEvent* event) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    const auto timerId = event->timerId();
    DEBUG_ASSERT(timerId != kInvalidTimerId);
    if (timerId != m_timeoutTimerId) {
        // ignore
        return;
    }
    killTimer(m_timeoutTimerId);
    m_timeoutTimerId = kInvalidTimerId;
    if (m_status != Status::Aborted) {
        m_status = Status::TimedOut;
    }
    kLogger.debug()
            << "Timed out";
    onTimedOut(doTimeOut());
}

QPair<QNetworkReply*, HttpStatusCode> WebTask::receiveNetworkReply() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(m_status != Status::Idle);
    auto* const networkReply = qobject_cast<QNetworkReply*>(sender());
    HttpStatusCode statusCode = kHttpStatusCodeInvalid;
    VERIFY_OR_DEBUG_ASSERT(networkReply) {
        return qMakePair(nullptr, statusCode);
    }
    networkReply->deleteLater();

    if (m_timeoutTimerId != kInvalidTimerId) {
        killTimer(m_timeoutTimerId);
        m_timeoutTimerId = kInvalidTimerId;
    }

    if (m_status == Status::Aborted) {
        onAborted(networkReply->request().url());
        return qMakePair(nullptr, statusCode);
    }
    m_status = Status::Finished;

    if (networkReply->error() != QNetworkReply::NetworkError::NoError) {
        onNetworkError(
                networkReply->request().url(),
                networkReply->error(),
                networkReply->errorString(),
                networkReply->readAll());
        return qMakePair(nullptr, statusCode);
    }

    if (kLogger.debugEnabled()) {
        if (networkReply->url() == networkReply->request().url()) {
            kLogger.debug()
                    << "Received reply for request"
                    << networkReply->url();
        } else {
            // Redirected
            kLogger.debug()
                    << "Received reply for redirected request"
                    << networkReply->request().url()
                    << "->"
                    << networkReply->url();
        }
    }

    DEBUG_ASSERT(statusCode == kHttpStatusCodeInvalid);
    VERIFY_OR_DEBUG_ASSERT(readStatusCode(networkReply, &statusCode)) {
        kLogger.warning()
                << "Failed to read HTTP status code";
    }

    return qMakePair(networkReply, statusCode);
}

} // namespace network

} // namespace mixxx
