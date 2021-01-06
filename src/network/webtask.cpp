#include "network/webtask.h"

#include <QTimerEvent>
#include <mutex> // std::once_flag

#include "moc_webtask.cpp"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace mixxx {

namespace network {

namespace {

const Logger kLogger("mixxx::network::WebTask");

constexpr int kInvalidTimerId = -1;

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
            << arg.m_replyUrl
            << arg.m_statusCode
            << '}';
}

/*static*/ void CustomWebResponse::registerMetaType() {
    qRegisterMetaType<CustomWebResponse>("mixxx::network::CustomWebResponse");
}

QDebug operator<<(QDebug dbg, const CustomWebResponse& arg) {
    return dbg
            << "CustomWebResponse{"
            << arg.m_response
            << arg.m_contentType
            << arg.m_contentBytes
            << '}';
}

WebTask::WebTask(
        QNetworkAccessManager* networkAccessManager,
        QObject* parent)
        : NetworkTask(networkAccessManager, parent),
          m_timeoutTimerId(kInvalidTimerId),
          m_state(State::Idle) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
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

    emitNetworkError(
            std::move(requestUrl),
            errorCode,
            std::move(errorString),
            std::move(errorContent));
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

    m_pendingNetworkReplyWeakPtr = doStartNetworkRequest(
            pNetworkAccessManager,
            timeoutMillis);
    // Still idle, because we are in the same thread.
    // The callee is not supposed to abort a request
    // before it has beeen started successfully.
    DEBUG_ASSERT(m_state == State::Idle);
    if (!m_pendingNetworkReplyWeakPtr) {
        kLogger.debug()
                << "Network task has not been started";
        return;
    }
    m_state = State::Pending;

    DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
    if (timeoutMillis != kNoTimeout) {
        DEBUG_ASSERT(timeoutMillis > 0);
        m_timeoutTimerId = startTimer(timeoutMillis);
        DEBUG_ASSERT(m_timeoutTimerId != kInvalidTimerId);
    }

    // It is not necessary to connect the QNetworkReply::errorOccurred signal.
    // Network errors are also received through the QNetworkReply::finished signal.
    connect(m_pendingNetworkReplyWeakPtr.data(),
            &QNetworkReply::finished,
            this,
            &WebTask::slotNetworkReplyFinished,
            Qt::UniqueConnection);
}

void WebTask::slotAbort() {
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
    emitAborted(requestUrl);
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
    // Triggering the regular abort workflow guarantees that
    // the internal state is switch into the intermediate state
    // State::Aborting when the request is still running!
    slotAbort();
}

void WebTask::slotNetworkReplyFinished() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    auto* const pFinishedNetworkReply = qobject_cast<QNetworkReply*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pFinishedNetworkReply) {
        return;
    }
    const auto finishedNetworkReplyDeleter = ScopedDeleteLater(pFinishedNetworkReply);

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
    VERIFY_OR_DEBUG_ASSERT(pPendingNetworkReply == pFinishedNetworkReply) {
        // Another or no reply is pending
        kLogger.warning()
                << this
                << "Discarding unexpected network reply:"
                << "finished =" << pFinishedNetworkReply
                << "pending =" << pPendingNetworkReply;
        return;
    }

    if (m_timeoutTimerId != kInvalidTimerId) {
        killTimer(m_timeoutTimerId);
        m_timeoutTimerId = kInvalidTimerId;
    }

    if (m_state != State::Pending) {
        kLogger.debug()
                << this
                << "Discarding obsolete network reply"
                << pFinishedNetworkReply;
        if (m_state == State::Aborting) {
            const auto requestUrl = pPendingNetworkReply->request().url();
            emitAborted(requestUrl);
        } else {
            // Might have been aborted or timed out in the meantime
            DEBUG_ASSERT(
                    m_state == State::Aborted ||
                    m_state == State::TimedOut);
        }
        return;
    }

    if (pFinishedNetworkReply->error() != QNetworkReply::NetworkError::NoError) {
        onNetworkError(
                pFinishedNetworkReply->request().url(),
                pFinishedNetworkReply->error(),
                pFinishedNetworkReply->errorString(),
                pFinishedNetworkReply->readAll());
        DEBUG_ASSERT(m_state != State::Pending);
        return;
    }

    m_state = State::Finished;
    const auto statusCode = readStatusCode(pFinishedNetworkReply);
    doNetworkReplyFinished(pFinishedNetworkReply, statusCode);
}

} // namespace network

} // namespace mixxx
