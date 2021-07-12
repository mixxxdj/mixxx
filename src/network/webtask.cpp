#include "network/webtask.h"

#include <QMimeDatabase>
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
    WebResponseWithContent::registerMetaType();
}

int readStatusCode(
        const QNetworkReply& networkReply) {
    const QVariant statusCodeAttr =
            networkReply.attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!statusCodeAttr.isValid()) {
        // No status code available
        return kHttpStatusCodeInvalid;
    }
    VERIFY_OR_DEBUG_ASSERT(statusCodeAttr.canConvert<int>()) {
        kLogger.warning()
                << "Invalid status code attribute"
                << statusCodeAttr;
        return kHttpStatusCodeInvalid;
    }
    bool statusCodeValid = false;
    const int statusCode = statusCodeAttr.toInt(&statusCodeValid);
    VERIFY_OR_DEBUG_ASSERT(statusCodeValid && HttpStatusCode_isValid(statusCode)) {
        kLogger.warning()
                << "Failed to read status code attribute"
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
            << arg.m_requestUrl
            << arg.m_replyUrl
            << arg.m_statusCode
            << '}';
}

/*static*/ void WebResponseWithContent::registerMetaType() {
    qRegisterMetaType<WebResponseWithContent>("mixxx::network::WebResponseWithContent");
}

QDebug operator<<(QDebug dbg, const WebResponseWithContent& arg) {
    return dbg
            << "WebResponseWithContent{"
            << arg.m_response
            << arg.m_contentType
            << arg.m_contentData
            << '}';
}

//static
QMimeType WebTask::readContentType(
        const QNetworkReply& reply) {
    const QVariant contentTypeHeader = reply.header(QNetworkRequest::ContentTypeHeader);
    if (!contentTypeHeader.isValid() || contentTypeHeader.isNull()) {
        if (reply.isReadable() && reply.bytesAvailable() > 0) {
            kLogger.warning()
                    << "Missing content type header";
        }
        return QMimeType();
    }
    const QString contentTypeString = contentTypeHeader.toString();
    const QString contentTypeWithoutParams = contentTypeString.left(contentTypeString.indexOf(';'));
    const QMimeType contentType = QMimeDatabase().mimeTypeForName(contentTypeWithoutParams);
    if (!contentType.isValid()) {
        kLogger.warning()
                << "Unknown content type"
                << contentTypeWithoutParams;
    }
    return contentType;
}

//static
std::optional<QByteArray> WebTask::readContentData(
        QNetworkReply* reply) {
    if (!reply->isReadable()) {
        return std::nullopt;
    }
    return reply->readAll();
}

WebTask::WebTask(
        QNetworkAccessManager* networkAccessManager,
        QObject* parent)
        : NetworkTask(networkAccessManager, parent),
          m_state(State::Idle),
          m_timeoutTimerId(kInvalidTimerId) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
}

void WebTask::onNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const WebResponseWithContent& responseWithContent) {
    DEBUG_ASSERT(m_state == State::Pending);
    DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);

    DEBUG_ASSERT(errorCode != QNetworkReply::NoError);
    switch (errorCode) {
    case QNetworkReply::OperationCanceledError:
        // Client-side abort or timeout
        m_state = State::Aborted;
        break;
    case QNetworkReply::TimeoutError:
        // Network or server-side timeout
        m_state = State::TimedOut;
        break;
    default:
        m_state = State::Failed;
    }
    DEBUG_ASSERT(hasTerminated());

    if (m_state == State::Aborted) {
        emitAborted(responseWithContent.requestUrl());
    } else {
        emitNetworkError(
                errorCode,
                errorString,
                responseWithContent);
    }
}

void WebTask::emitNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const WebResponseWithContent& responseWithContent) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&WebTask::networkError)) {
        kLogger.warning()
                << this
                << "Unhandled network error signal"
                << errorCode
                << errorString
                << responseWithContent;
        deleteLater();
        return;
    }
    emit networkError(
            errorCode,
            errorString,
            responseWithContent);
}

void WebTask::slotStart(int timeoutMillis) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_state == State::Pending) {
        kLogger.warning()
                << "Task is still busy and cannot be started again";
        return;
    }

    // Reset state
    DEBUG_ASSERT(!m_pendingNetworkReplyWeakPtr);
    DEBUG_ASSERT(m_timeoutTimerId == kInvalidTimerId);
    m_state = State::Idle;

    auto* const pNetworkAccessManager = m_networkAccessManagerWeakPtr.data();
    VERIFY_OR_DEBUG_ASSERT(pNetworkAccessManager) {
        m_state = State::Pending;
        onNetworkError(
                QNetworkReply::NetworkSessionFailedError,
                tr("No network access"),
                WebResponseWithContent{});
        return;
    }
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(pNetworkAccessManager);

    kLogger.debug()
            << this
            << "Starting...";
    m_timer.start();

    m_pendingNetworkReplyWeakPtr = doStartNetworkRequest(
            pNetworkAccessManager,
            timeoutMillis);
    // Still idle, because we are in the same thread.
    // The derived class is not allowed to abort a request
    // during the callback before it has beeen started
    // successfully. Instead it should return nullptr
    // to abort the task immediately.
    DEBUG_ASSERT(m_state == State::Idle);
    if (!m_pendingNetworkReplyWeakPtr) {
        kLogger.debug()
                << this
                << "Network request has not been started";
        m_state = State::Aborted;
        emitAborted(/*request URL is unknown*/);
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
        if (m_state == State::Idle) {
            kLogger.debug()
                    << this
                    << "Cannot abort idle task";
        } else {
            DEBUG_ASSERT(hasTerminated());
            kLogger.debug()
                    << this
                    << "Cannot abort terminated task";
        }
        return;
    }

    kLogger.debug()
            << this
            << "Aborting...";

    if (m_timeoutTimerId != kInvalidTimerId) {
        killTimer(m_timeoutTimerId);
        m_timeoutTimerId = kInvalidTimerId;
    }

    auto* const pPendingNetworkReply = m_pendingNetworkReplyWeakPtr.data();
    QUrl requestUrl;
    if (pPendingNetworkReply) {
        if (pPendingNetworkReply->isRunning()) {
            kLogger.debug()
                    << this
                    << "Aborting pending network reply after"
                    << m_timer.elapsed().toIntegerMillis()
                    << "ms";
            pPendingNetworkReply->abort();
            // Aborting a pending reply will immediately emit a network
            // error signal that gets handled in this thread before
            // continuing with the next statements.
            DEBUG_ASSERT(hasTerminated());
            DEBUG_ASSERT(!m_pendingNetworkReplyWeakPtr);
            return;
        }
        kLogger.debug()
                << this
                << "Aborted pending network reply after"
                << m_timer.elapsed().toIntegerMillis()
                << "ms";
        // Save the request URL for emitting the signal (see below)
        requestUrl = pPendingNetworkReply->request().url();
        // Ensure that the aborted reply is scheduled for deletion when leaving
        // this scope.
        const auto pendingNetworkReplyDeleter = ScopedDeleteLater(pPendingNetworkReply);
        m_pendingNetworkReplyWeakPtr.clear();
        doNetworkReplyAborted(pPendingNetworkReply);
    }

    m_state = State::Aborted;
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

    if (hasTerminated()) {
        DEBUG_ASSERT(!m_pendingNetworkReplyWeakPtr);
        return;
    }
    DEBUG_ASSERT(m_state == State::Pending);

    kLogger.info()
            << this
            << "Aborting after timed out";
    // Trigger the regular abort workflow after a client-side
    // timeout occurred
    slotAbort();
}

void WebTask::slotNetworkReplyFinished() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);

    auto* const pFinishedNetworkReply = qobject_cast<QNetworkReply*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pFinishedNetworkReply) {
        return;
    }
    // Ensure that the received reply gets deleted eventually
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
    m_pendingNetworkReplyWeakPtr.clear();

    DEBUG_ASSERT(m_state == State::Pending);
    kLogger.debug()
            << this
            << "Received network reply after"
            << m_timer.elapsed().toIntegerMillis()
            << "ms";

    if (m_timeoutTimerId != kInvalidTimerId) {
        killTimer(m_timeoutTimerId);
        m_timeoutTimerId = kInvalidTimerId;
    }

    const auto statusCode = readStatusCode(*pFinishedNetworkReply);
    if (pFinishedNetworkReply->error() != QNetworkReply::NetworkError::NoError) {
        onNetworkError(
                pFinishedNetworkReply->error(),
                pFinishedNetworkReply->errorString(),
                WebResponseWithContent{
                        WebResponse{
                                pFinishedNetworkReply->url(),
                                pFinishedNetworkReply->request().url(),
                                statusCode},
                        readContentType(*pFinishedNetworkReply),
                        readContentData(pFinishedNetworkReply).value_or(QByteArray{}),
                });
        DEBUG_ASSERT(hasTerminated());
        return;
    }

    m_state = State::Finished;
    doNetworkReplyFinished(pFinishedNetworkReply, statusCode);
}

} // namespace network

} // namespace mixxx
