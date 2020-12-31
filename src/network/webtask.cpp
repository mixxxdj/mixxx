#include "network/webtask.h"

#include <QTimerEvent>
#include <mutex> // std::once_flag

#include "moc_webtask.cpp"
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

int readStatusCode(
        const QNetworkReply* networkReply) {
    const QVariant statusCodeAttr =
            networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    bool statusCodeValid = false;
    const int statusCode = statusCodeAttr.toInt(&statusCodeValid);
    VERIFY_OR_DEBUG_ASSERT(statusCodeValid && HttpStatusCode_isValid(statusCode)) {
        kLogger.warning()
                << "Invalid or missing status code attribute"
                << statusCodeAttr;
        return kHttpStatusCodeInvalid;
    }
    return statusCode;
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
          m_networkAccessManagerWeakPtr(networkAccessManager),
          m_timeoutTimerId(kInvalidTimerId),
          m_state(State::Idle) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
    s_instanceCounter.increment(1);
}

WebTask::~WebTask() {
    s_instanceCounter.increment(-1);
}

void WebTask::onNetworkError(
        QUrl&& requestUrl,
        QNetworkReply::NetworkError errorCode,
        QString&& errorString,
        QByteArray&& errorContent) {
    DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
    VERIFY_OR_DEBUG_ASSERT(m_state == State::Pending) {
        return;
    }

    DEBUG_ASSERT(errorCode != QNetworkReply::NoError);
    switch (errorCode) {
    case QNetworkReply::OperationCanceledError:
        m_state = State::Aborted;
        break;
    case QNetworkReply::TimeoutError:
        m_state = State::TimedOut;
        break;
    default:
        m_state = State::Failed;
    }

    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&WebTask::networkError)) {
        kLogger.warning()
                << this
                << "Unhandled network error:"
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
    VERIFY_OR_DEBUG_ASSERT(m_state != State::Pending) {
        return;
    }
    DEBUG_ASSERT(!m_pendingNetworkReplyWeakPtr);
    m_state = State::Idle;

    auto* const pNetworkAccessManager = m_networkAccessManagerWeakPtr.data();
    VERIFY_OR_DEBUG_ASSERT(pNetworkAccessManager) {
        m_state = State::Pending;
        onNetworkError(
                QUrl(),
                QNetworkReply::NetworkSessionFailedError,
                tr("No network access"),
                QByteArray());
        return;
    }
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(pNetworkAccessManager);

    kLogger.debug()
            << this
            << "Starting...";

    auto* const pPendingNetworkReply = doStartNetworkRequest(
            pNetworkAccessManager,
            timeoutMillis);
    // Still idle, because we are in the same thread.
    // The callee is not supposed to abort a request
    // before it has beeen started successfully.
    DEBUG_ASSERT(m_state == State::Idle);
    if (!pPendingNetworkReply) {
        kLogger.debug()
                << "Network task has not been started";
        return;
    }
    m_state = State::Pending;

    DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
    if (timeoutMillis > 0) {
        m_timeoutTimerId = startTimer(timeoutMillis);
        DEBUG_ASSERT(m_timeoutTimerId != kInvalidTimerId);
    }

    // It is not necessary to connect the QNetworkReply::errorOccurred signal.
    // Network errors are also received through the QNetworkReply::finished signal.
    connect(pPendingNetworkReply,
            &QNetworkReply::finished,
            this,
            &WebTask::slotNetworkReplyFinished,
            Qt::UniqueConnection);

    m_pendingNetworkReplyWeakPtr = pPendingNetworkReply;
}

void WebTask::abort() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_state != State::Pending) {
        DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
        return;
    }
    auto* const pPendingNetworkReply = m_pendingNetworkReplyWeakPtr.data();
    VERIFY_OR_DEBUG_ASSERT(pPendingNetworkReply) {
        DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
        return;
    }
    if (m_timeoutTimerId != kInvalidTimerId) {
        killTimer(m_timeoutTimerId);
        m_timeoutTimerId = kInvalidTimerId;
    }
    m_state = State::Aborting;
    kLogger.debug()
            << this
            << "Aborting...";
    if (pPendingNetworkReply->isRunning()) {
        pPendingNetworkReply->abort();
        doNetworkReplyAborted(pPendingNetworkReply);
        // Suspend and await finished signal
        return;
    }
    doNetworkReplyAborted(pPendingNetworkReply);
    m_state = State::Aborted;
    const auto requestUrl = pPendingNetworkReply->request().url();
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&WebTask::aborted)) {
        kLogger.warning()
                << this
                << "Unhandled abort signal"
                << requestUrl;
        deleteLater();
        return;
    }
    emit aborted(
            std::move(requestUrl));
}

void WebTask::slotAbort() {
    abort();
}

void WebTask::timerEvent(QTimerEvent* event) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    const auto timerId = event->timerId();
    DEBUG_ASSERT(timerId != kInvalidTimerId);
    VERIFY_OR_DEBUG_ASSERT(timerId == m_timeoutTimerId) {
        return;
    }
    killTimer(m_timeoutTimerId);
    m_timeoutTimerId = kInvalidTimerId;
    VERIFY_OR_DEBUG_ASSERT(m_state == State::Pending) {
        return;
    }
    auto* const pPendingNetworkReply = m_pendingNetworkReplyWeakPtr.data();
    VERIFY_OR_DEBUG_ASSERT(pPendingNetworkReply) {
        return;
    }
    if (pPendingNetworkReply->isFinished()) {
        // Nothing to do
    }
    kLogger.info()
            << this
            << "Aborting after timed out";
    DEBUG_ASSERT(pPendingNetworkReply->isRunning());
    pPendingNetworkReply->abort();
}

void WebTask::slotNetworkReplyFinished() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    auto* const pFinishedNetworkReply = qobject_cast<QNetworkReply*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pFinishedNetworkReply) {
        return;
    }
    if (kLogger.debugEnabled()) {
        if (pFinishedNetworkReply->url() == pFinishedNetworkReply->request().url()) {
            kLogger.debug()
                    << this
                    << "Received reply for request"
                    << pFinishedNetworkReply->url();
        } else {
            // Redirected
            kLogger.debug()
                    << this
                    << "Received reply for redirected request"
                    << pFinishedNetworkReply->request().url()
                    << "->"
                    << pFinishedNetworkReply->url();
        }
    }

    // Check correlation between pending and finished reply
    auto* const pPendingNetworkReply = m_pendingNetworkReplyWeakPtr.data();
    if (!pPendingNetworkReply) {
        // No reply is pending
        DEBUG_ASSERT(m_state == State::Aborted || m_state == State::TimedOut);
        DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
        kLogger.debug()
                << this
                << "Discarding obsolete network reply";
        pFinishedNetworkReply->deleteLater();
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(pPendingNetworkReply == pFinishedNetworkReply) {
        // Another reply is pending
        kLogger.debug()
                << this
                << "Discarding unexpected network reply";
        pFinishedNetworkReply->deleteLater();
        return;
    }
    m_pendingNetworkReplyWeakPtr = nullptr;

    pFinishedNetworkReply->deleteLater();

    VERIFY_OR_DEBUG_ASSERT(m_state == State::Pending) {
        DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
        return;
    }

    if (m_timeoutTimerId != kInvalidTimerId) {
        killTimer(m_timeoutTimerId);
        m_timeoutTimerId = kInvalidTimerId;
    }

    if (pFinishedNetworkReply->error() != QNetworkReply::NetworkError::NoError) {
        onNetworkError(
                pFinishedNetworkReply->request().url(),
                pFinishedNetworkReply->error(),
                pFinishedNetworkReply->errorString(),
                pFinishedNetworkReply->readAll());
        return;
    }

    m_state = State::Finished;
    const auto statusCode = readStatusCode(pFinishedNetworkReply);
    doNetworkReplyFinished(pFinishedNetworkReply, statusCode);
}

} // namespace network

} // namespace mixxx
