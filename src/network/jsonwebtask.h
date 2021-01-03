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

struct JsonWebResponse : public WebResponse {
  public:
    static void registerMetaType();

    JsonWebResponse() = default;
    JsonWebResponse(
            WebResponse response,
            QJsonDocument content)
            : WebResponse(std::move(response)),
              content(std::move(content)) {
    }
    JsonWebResponse(const JsonWebResponse&) = default;
    JsonWebResponse(JsonWebResponse&&) = default;
    ~JsonWebResponse() override = default;

    JsonWebResponse& operator=(const JsonWebResponse&) = default;
    JsonWebResponse& operator=(JsonWebResponse&&) = default;

    QJsonDocument content;
};

QDebug operator<<(QDebug dbg, const JsonWebResponse& arg);

class JsonWebTask : public WebTask {
    Q_OBJECT

  public:
    JsonWebTask(
            QNetworkAccessManager* networkAccessManager,
            const QUrl& baseUrl,
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
            network::JsonWebResponse&& response);

  private:
    // Handle the response and ensure that the task eventually
    // gets deleted. The default implementation discards the
    // response and deletes the task.
    virtual void onFinished(
            JsonWebResponse&& response);
    virtual void onFinishedCustom(
            CustomWebResponse&& response);

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
