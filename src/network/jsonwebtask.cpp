#include "network/jsonwebtask.h"

#include <QMetaMethod>
#include <QMimeDatabase>
#include <QNetworkRequest>
#include <QTimerEvent>
#include <mutex> // std::once_flag

#include "moc_jsonwebtask.cpp"
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

/// If parsing fails the functions returns std::nullopt and optionally
/// the response content in pInvalidResponseContent for further processing.
std::optional<QJsonDocument> readJsonContent(
        QNetworkReply* reply,
        std::pair<QMimeType, QByteArray>* pInvalidResponseContent = nullptr) {
    DEBUG_ASSERT(reply);
    DEBUG_ASSERT(JSON_MIME_TYPE.isValid());
    auto contentType = WebTask::readContentType(*reply);
    auto optContentData = WebTask::readContentData(reply);
    if (contentType != JSON_MIME_TYPE) {
        kLogger.debug()
                << "Received content type"
                << contentType
                << "instead of"
                << JSON_MIME_TYPE;
        if (pInvalidResponseContent) {
            *pInvalidResponseContent = std::make_pair(
                    std::move(contentType),
                    optContentData.value_or(QByteArray{}));
        }
        return std::nullopt;
    }
    if (!optContentData || optContentData->isEmpty()) {
        kLogger.debug() << "Empty JSON network reply";
        return QJsonDocument{};
    }
    QJsonParseError parseError;
    auto jsonDocument = QJsonDocument::fromJson(
            *optContentData,
            &parseError);
    // QJsonDocument::fromJson() returns a non-null document
    // if parsing succeeds and otherwise null on error. The
    // parse error must only be evaluated if the returned
    // document is null!
    if (jsonDocument.isNull() &&
            parseError.error != QJsonParseError::NoError) {
        kLogger.warning()
                << "Failed to parse JSON data:"
                << parseError.errorString()
                << "at offset"
                << parseError.offset;
        if (pInvalidResponseContent) {
            *pInvalidResponseContent = std::make_pair(
                    std::move(contentType),
                    std::move(*optContentData));
        }
        return std::nullopt;
    }
    return jsonDocument;
}

// TODO: Allow to customize headers and attributes?
QNetworkRequest newRequest(
        const QUrl& url) {
    QNetworkRequest request(url);
    request.setHeader(
            QNetworkRequest::ContentTypeHeader,
            JSON_CONTENT_TYPE);
    request.setAttribute(
            QNetworkRequest::RedirectPolicyAttribute,
            QNetworkRequest::NoLessSafeRedirectPolicy);
    return request;
}

} // anonymous namespace

/*static*/ void JsonWebResponse::registerMetaType() {
    qRegisterMetaType<JsonWebResponse>("mixxx::network::JsonWebResponse");
}

QDebug operator<<(QDebug dbg, const JsonWebResponse& arg) {
    return dbg
            << "WebResponseWithContent{"
            << arg.m_response
            << arg.m_content
            << '}';
}

JsonWebTask::JsonWebTask(
        QNetworkAccessManager* networkAccessManager,
        QUrl baseUrl,
        JsonWebRequest&& request,
        QObject* parent)
        : WebTask(networkAccessManager, parent),
          m_baseUrl(std::move(baseUrl)),
          m_request(request) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypesOnce);
    DEBUG_ASSERT(!m_baseUrl.isEmpty());
}

void JsonWebTask::onFinished(
        const JsonWebResponse& jsonResponse) {
    kLogger.info()
            << this
            << "Received JSON response"
            << jsonResponse;
    deleteLater();
}

void JsonWebTask::onFinishedCustom(
        const WebResponseWithContent& customResponse) {
    kLogger.info()
            << this
            << "Received custom response"
            << customResponse;
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
                    << this
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
                    << this
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
                    << this
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
                    << this
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
                    << this
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

QNetworkReply* JsonWebTask::doStartNetworkRequest(
        QNetworkAccessManager* networkAccessManager,
        int parentTimeoutMillis) {
    Q_UNUSED(parentTimeoutMillis);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(networkAccessManager);

    VERIFY_OR_DEBUG_ASSERT(m_baseUrl.isValid()) {
        kLogger.warning() << "Invalid base URL" << m_baseUrl;
        return nullptr;
    }
    QUrl url = m_baseUrl;
    url.setPath(m_request.path);
    VERIFY_OR_DEBUG_ASSERT(url.isValid()) {
        kLogger.warning() << "Invalid request path" << m_request.path;
        return nullptr;
    }
    if (!m_request.query.isEmpty()) {
        url.setQuery(m_request.query);
        VERIFY_OR_DEBUG_ASSERT(url.isValid()) {
            kLogger.warning() << "Invalid query string" << m_request.query.toString();
            return nullptr;
        }
    }
    // Already validated while composing (see above)
    DEBUG_ASSERT(url.isValid());

    return sendNetworkRequest(
            networkAccessManager,
            m_request.method,
            url,
            m_request.content);
}

void JsonWebTask::doNetworkReplyFinished(
        QNetworkReply* finishedNetworkReply,
        HttpStatusCode statusCode) {
    DEBUG_ASSERT(finishedNetworkReply);
    std::optional<QJsonDocument> optJsonContent;
    auto webResponse = WebResponse{
            finishedNetworkReply->url(),
            finishedNetworkReply->request().url(),
            statusCode};
    if (statusCode != kHttpStatusCodeInvalid) {
        std::pair<QMimeType, QByteArray> contentTypeAndBytes;
        optJsonContent = readJsonContent(finishedNetworkReply, &contentTypeAndBytes);
        if (!optJsonContent) {
            // Failed to read JSON content
            onFinishedCustom(WebResponseWithContent{
                    std::move(webResponse),
                    std::move(contentTypeAndBytes.first),
                    std::move(contentTypeAndBytes.second)});
            return;
        }
    }
    onFinished(JsonWebResponse{
            std::move(webResponse),
            optJsonContent.value_or(QJsonDocument{})});
}

void JsonWebTask::emitFailed(
        const network::JsonWebResponse& response) {
    VERIFY_OR_DEBUG_ASSERT(
            isSignalFuncConnected(&JsonWebTask::failed)) {
        kLogger.warning()
                << this
                << "Unhandled failed signal"
                << response;
        deleteLater();
        return;
    }
    emit failed(response);
}

} // namespace network

} // namespace mixxx
