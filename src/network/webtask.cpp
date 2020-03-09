#include "network/webtask.h"

#include <QMetaMethod>
#include <QThread>
#include <QTimerEvent>
#include <mutex> // std::once_flag

#include "util/assert.h"
#include "util/counter.h"
#include "util/logger.h"

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

/*static*/ void CustomWebResponse::registerMetaType() {
    qRegisterMetaType<CustomWebResponse>("mixxx::network::CustomWebResponse");
}

WebTask::WebTask(
        QNetworkAccessManager* networkAccessManager,
        QObject* parent)
        : QObject(parent),
          m_networkAccessManager(networkAccessManager),
          m_timeoutTimerId(kInvalidTimerId),
          m_abort(false) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
    DEBUG_ASSERT(m_networkAccessManager);
    s_instanceCounter.increment(1);
}

WebTask::~WebTask() {
    s_instanceCounter.increment(-1);
}

void WebTask::onAborted() {
    DEBUG_ASSERT(m_abort);
    const auto signal = QMetaMethod::fromSignal(
            &WebTask::aborted);
    if (isSignalConnected(signal)) {
        emit aborted();
    } else {
        kLogger.info()
                << "Request aborted";
        deleteAfterFinished();
    }
}

void WebTask::onNetworkError(
        QUrl requestUrl,
        QNetworkReply::NetworkError errorCode,
        QString errorString,
        QByteArray errorContent) {
    const auto signal = QMetaMethod::fromSignal(
            &WebTask::networkError);
    if (isSignalConnected(signal)) {
        emit networkError(
                std::move(requestUrl),
                errorCode,
                std::move(errorString),
                std::move(errorContent));
    } else {
        kLogger.warning()
                << "Network error"
                << requestUrl
                << errorCode
                << errorString
                << errorContent;
        deleteAfterFinished();
    }
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

void WebTask::deleteBeforeFinished() {
    // Might be called from any thread so we must not
    // access any member variables!
    // Do not disconnect any connections, because otherwise
    // the destroyed() signal is not received!
    invokeAbort();
    deleteLater();
}

void WebTask::deleteAfterFinished() {
    // Might be called from any thread so we must not
    // access any member variables!
    // Do not disconnect any connections, because otherwise
    // the destroyed() signal is not received!
    deleteLater();
}

void WebTask::slotStart(int timeoutMillis) {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    VERIFY_OR_DEBUG_ASSERT(m_networkAccessManager) {
        kLogger.warning()
                << "No network access";
        return;
    }
    // The task might be restarted after it has been aborted
    // or finished.
    m_abort = false;

    kLogger.debug()
            << "Starting...";
    if (!doStart(m_networkAccessManager, timeoutMillis)) {
        kLogger.warning()
                << "Start aborted";
        return;
    }

    // The task could be aborted immediately while being started.
    if (m_abort) {
        onAborted();
        return;
    }

    DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
    if (timeoutMillis > 0) {
        m_timeoutTimerId = startTimer(timeoutMillis);
        DEBUG_ASSERT(m_timeoutTimerId != kInvalidTimerId);
    }
}

void WebTask::slotAbort() {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    if (m_abort) {
        DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
        return;
    }
    if (m_timeoutTimerId != kInvalidTimerId) {
        killTimer(m_timeoutTimerId);
        m_timeoutTimerId = kInvalidTimerId;
    }
    m_abort = true;
    kLogger.debug()
            << "Aborting...";
    doAbort();
}

void WebTask::timerEvent(QTimerEvent* event) {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    const auto timerId = event->timerId();
    DEBUG_ASSERT(timerId != kInvalidTimerId);
    if (timerId != m_timeoutTimerId) {
        // ignore
        return;
    }
    kLogger.info()
            << "Timed out";
    slotAbort();
}

QPair<QNetworkReply*, HttpStatusCode> WebTask::receiveNetworkReply() {
    DEBUG_ASSERT(thread() == QThread::currentThread());
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

    if (m_abort) {
        onAborted();
        return qMakePair(nullptr, statusCode);
    }

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
