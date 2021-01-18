#pragma once

#include <QJsonDocument>
#include <QUrlQuery>

#include "network/httprequestmethod.h"
#include "network/webtask.h"

namespace mixxx {

namespace network {

struct JsonWebRequest final {
  public:
    JsonWebRequest() = delete;
    JsonWebRequest(const JsonWebRequest&) = default;
    JsonWebRequest(JsonWebRequest&&) = default;

    JsonWebRequest& operator=(const JsonWebRequest&) = default;
    JsonWebRequest& operator=(JsonWebRequest&&) = default;

    HttpRequestMethod method;
    QString path;
    QUrlQuery query;
    QJsonDocument content;
};

class JsonWebResponse final {
  public:
    static void registerMetaType();

    JsonWebResponse() = default;
    JsonWebResponse(
            WebResponse&& response,
            QJsonDocument&& content)
            : m_response(response),
              m_content(content) {
    }
    JsonWebResponse(const JsonWebResponse&) = default;
    JsonWebResponse(JsonWebResponse&&) = default;

    JsonWebResponse& operator=(const JsonWebResponse&) = default;
    JsonWebResponse& operator=(JsonWebResponse&&) = default;

    bool isStatusCodeSuccess() const {
        return m_response.isStatusCodeSuccess();
    }

    HttpStatusCode statusCode() const {
        return m_response.statusCode();
    }

    const QUrl& replyUrl() const {
        return m_response.replyUrl();
    }

    const QJsonDocument& content() const {
        return m_content;
    }

    friend QDebug operator<<(QDebug dbg, const JsonWebResponse& arg);

  private:
    WebResponse m_response;
    QJsonDocument m_content;
};

class JsonWebTask : public WebTask {
    Q_OBJECT

  public:
    JsonWebTask(
            QNetworkAccessManager* networkAccessManager,
            QUrl baseUrl,
            JsonWebRequest&& request,
            QObject* parent = nullptr);
    ~JsonWebTask() override = default;

  signals:
    void failed(
            const network::JsonWebResponse& response);

  protected:
    // Customizable in derived classes
    virtual QNetworkReply* sendNetworkRequest(
            QNetworkAccessManager* networkAccessManager,
            HttpRequestMethod method,
            const QUrl& url,
            const QJsonDocument& content);

    void emitFailed(
            const network::JsonWebResponse& response);

  private:
    /// Handle the response and ensure that the task eventually
    /// gets deleted.
    ///
    /// Could be overridden by derived classes. The default
    /// implementation discards the response and deletes the task.
    virtual void onFinished(
            const JsonWebResponse& jsonResponse);
    virtual void onFinishedCustom(
            const WebResponseWithContent& customResponse);

    QNetworkReply* doStartNetworkRequest(
            QNetworkAccessManager* networkAccessManager,
            int parentTimeoutMillis) override;
    void doNetworkReplyFinished(
            QNetworkReply* finishedNetworkReply,
            HttpStatusCode statusCode) override;

    // All member variables must only be accessed from
    // the event loop thread!!
    const QUrl m_baseUrl;
    const JsonWebRequest m_request;
};

} // namespace network

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::network::JsonWebResponse);
