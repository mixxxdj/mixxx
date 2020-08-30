#include "network/jsonwebtask.h"

#include <QMetaMethod>
#include <QMimeDatabase>
#include <QNetworkRequest>
#include <QTimerEvent>
#include <mutex> // std::once_flag

#include "util/counter.h"
#include "util/logger.h"
#include "util/thread_affinity.h"

namespace mixxx {

namespace network {

namespace {

const Logger kLogger("mixxx::network::JsonWebTask");

std::once_flag registerMetaTypesOnceFlag;

void registerMetaTypesOnce() {
    JsonWebResponse::registerMetaType();
}

const QString JSON_CONTENT_TYPE = "application/json";

const QMimeType JSON_MIME_TYPE = QMimeDatabase().mimeTypeForName(JSON_CONTENT_TYPE);

QMimeType readContentType(
        const QNetworkReply* reply) {
    DEBUG_ASSERT(reply);
    const QVariant contentTypeHeader = reply->header(QNetworkRequest::ContentTypeHeader);
    if (!contentTypeHeader.isValid() || contentTypeHeader.isNull()) {
        kLogger.warning()
                << "Missing content type header";
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

bool readJsonContent(
        QNetworkReply* reply,
        QJsonDocument* jsonContent) {
    DEBUG_ASSERT(reply);
    DEBUG_ASSERT(jsonContent);
    DEBUG_ASSERT(JSON_MIME_TYPE.isValid());
    const auto contentType = readContentType(reply);
    if (contentType != JSON_MIME_TYPE) {
        kLogger.warning()
                << "Unexpected content type"
                << contentType;
        return false;
    }
    QByteArray jsonData = reply->readAll();
    if (jsonData.isEmpty()) {
        kLogger.warning()
                << "Empty reply";
        return false;
    }
    QJsonParseError parseError;
    const auto jsonDoc = QJsonDocument::fromJson(
            jsonData,
            &parseError);
    // QJsonDocument::fromJson() returns a non-null document
    // if parsing succeeds and otherwise null on error. The
    // parse error must only be evaluated if the returned
    // document is null!
    if (jsonDoc.isNull() &&
            parseError.error != QJsonParseError::NoError) {
        kLogger.warning()
                << "Failed to parse JSON data:"
                << parseError.errorString()
                << "at offset"
                << parseError.offset;
        return false;
    }
    *jsonContent = jsonDoc;
    return true;
}

// TODO: Allow to customize headers and attributes?
QNetworkRequest newRequest(
        const QUrl& url) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, JSON_CONTENT_TYPE);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    return request;
}

} // anonymous namespace

/*static*/ void JsonWebResponse::registerMetaType() {
    qRegisterMetaType<JsonWebResponse>("mixxx::network::JsonWebResponse");
}

QDebug operator<<(QDebug dbg, const JsonWebResponse& arg) {
    return dbg
        << "CustomWebResponse{"
        << static_cast<const WebResponse&>(arg)
        << arg.content
        << '}';
}

JsonWebTask::JsonWebTask(
        QNetworkAccessManager* networkAccessManager,
        const QUrl& baseUrl,
        JsonWebRequest&& request,
        QObject* parent)
        : WebTask(networkAccessManager, parent),
          m_baseUrl(baseUrl),
          m_request(std::move(request)),
          m_pendingNetworkReply(nullptr) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
    DEBUG_ASSERT(!m_baseUrl.isEmpty());
}

JsonWebTask::~JsonWebTask() {
    if (m_pendingNetworkReply) {
        m_pendingNetworkReply->deleteLater();
    }
}

void JsonWebTask::onFinished(
        JsonWebResponse&& response) {
    kLogger.info()
            << "Response received"
            << response.replyUrl
            << response.statusCode
            << response.content;
    deleteLater();
}

void JsonWebTask::onFinishedCustom(
        CustomWebResponse&& response) {
    kLogger.info()
            << "Custom response received"
            << response.replyUrl
            << response.statusCode
            << response.content;
    deleteLater();
}

QNetworkReply* JsonWebTask::sendNetworkRequest(
        QNetworkAccessManager* networkAccessManager,
        HttpRequestMethod method,
        const QUrl& url,
        const QJsonDocument& content) {
    switch (method) {
    case HttpRequestMethod::Get: {
        DEBUG_ASSERT(m_request.content.isEmpty());
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "GET"
                    << url;
        }
        return networkAccessManager->get(
                QNetworkRequest(url));
    }
    case HttpRequestMethod::Put: {
        const auto body = content.toJson(QJsonDocument::Compact);
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "PUT"
                    << url
                    << body;
        }
        return networkAccessManager->put(
                newRequest(url),
                body);
    }
    case HttpRequestMethod::Post: {
        const auto body = content.toJson(QJsonDocument::Compact);
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "POST"
                    << url
                    << body;
        }
        return networkAccessManager->post(
                newRequest(url),
                body);
    }
    case HttpRequestMethod::Patch: {
        const auto body = m_request.content.toJson(QJsonDocument::Compact);
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "PATCH"
                    << url
                    << body;
        }
        return networkAccessManager->sendCustomRequest(
                newRequest(url),
                "PATCH",
                body);
    }
    case HttpRequestMethod::Delete: {
        DEBUG_ASSERT(content.isEmpty());
        if (kLogger.debugEnabled()) {
            kLogger.debug()
                    << "DELETE"
                    << url;
        }
        return networkAccessManager->deleteResource(
                QNetworkRequest(url));
    }
    }
    DEBUG_ASSERT(!"unreachable");
    return nullptr;
}

bool JsonWebTask::doStart(
        QNetworkAccessManager* networkAccessManager,
        int parentTimeoutMillis) {
    Q_UNUSED(parentTimeoutMillis);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(networkAccessManager);
    VERIFY_OR_DEBUG_ASSERT(!m_pendingNetworkReply) {
        kLogger.warning()
                << "Task has already been started";
        return false;
    }

    DEBUG_ASSERT(m_baseUrl.isValid());
    QUrl url = m_baseUrl;
    url.setPath(m_request.path);
    if (!m_request.query.isEmpty()) {
        url.setQuery(m_request.query);
    }
    DEBUG_ASSERT(url.isValid());

    m_pendingNetworkReply = sendNetworkRequest(
            networkAccessManager,
            m_request.method,
            url,
            m_request.content);
    VERIFY_OR_DEBUG_ASSERT(m_pendingNetworkReply) {
        kLogger.warning()
                << "Request not sent";
        return false;
    }

    connect(m_pendingNetworkReply,
            &QNetworkReply::finished,
            this,
            &JsonWebTask::slotNetworkReplyFinished,
            Qt::UniqueConnection);

    connect(m_pendingNetworkReply,
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            &QNetworkReply::errorOccurred,
#else
            QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
#endif
            this,
            &JsonWebTask::slotNetworkReplyFinished,
            Qt::UniqueConnection);

    return true;
}

QUrl JsonWebTask::doAbort() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    QUrl requestUrl;
    if (m_pendingNetworkReply) {
        requestUrl = abortPendingNetworkReply(m_pendingNetworkReply);
        if (requestUrl.isValid()) {
            // Already finished
            m_pendingNetworkReply->deleteLater();
            m_pendingNetworkReply = nullptr;
        }
    }
    return requestUrl;
}

QUrl JsonWebTask::doTimeOut() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    QUrl requestUrl;
    if (m_pendingNetworkReply) {
        requestUrl = timeOutPendingNetworkReply(m_pendingNetworkReply);
        // Don't wait until finished
        m_pendingNetworkReply->deleteLater();
        m_pendingNetworkReply = nullptr;
    }
    return requestUrl;
}

void JsonWebTask::slotNetworkReplyFinished() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    const QPair<QNetworkReply*, HttpStatusCode> networkReplyWithStatusCode =
            receiveNetworkReply();
    auto* const networkReply = networkReplyWithStatusCode.first;
    if (!networkReply) {
        // already aborted
        return;
    }
    const auto statusCode = networkReplyWithStatusCode.second;
    VERIFY_OR_DEBUG_ASSERT(networkReply == m_pendingNetworkReply) {
        return;
    }
    m_pendingNetworkReply = nullptr;

    QJsonDocument content;
    if (statusCode != kHttpStatusCodeInvalid &&
            networkReply->bytesAvailable() > 0 &&
            !readJsonContent(networkReply, &content)) {
        onFinishedCustom(CustomWebResponse{
                WebResponse{
                        networkReply->url(),
                        statusCode},
                networkReply->readAll()});
    } else {
        onFinished(JsonWebResponse{
                WebResponse{
                        networkReply->url(),
                        statusCode},
                std::move(content)});
    }
}

void JsonWebTask::emitFailed(
        network::JsonWebResponse&& response) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&JsonWebTask::failed)) {
        kLogger.warning()
                << "Unhandled failed signal"
                << response;
        deleteLater();
        return;
    }
    emit failed(std::move(response));
}

} // namespace network

} // namespace mixxx
