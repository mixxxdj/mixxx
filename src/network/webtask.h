#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QUrl>

#include "network/httpstatuscode.h"
#include "network/networktask.h"
#include "util/optional.h"

namespace mixxx {

namespace network {

struct WebResponse {
  public:
    static void registerMetaType();

    WebResponse()
            : statusCode(kHttpStatusCodeInvalid) {
    }
    explicit WebResponse(
            QUrl replyUrl,
            HttpStatusCode statusCode = kHttpStatusCodeInvalid)
            : replyUrl(std::move(replyUrl)),
              statusCode(statusCode) {
    }
    WebResponse(const WebResponse&) = default;
    WebResponse(WebResponse&&) = default;
    virtual ~WebResponse() = default;

    WebResponse& operator=(const WebResponse&) = default;
    WebResponse& operator=(WebResponse&&) = default;

    bool isStatusCodeValid() const {
        return HttpStatusCode_isValid(statusCode);
    }
    bool isStatusCodeSuccess() const {
        return HttpStatusCode_isSuccess(statusCode);
    }
    bool isStatusCodeRedirection() const {
        return HttpStatusCode_isRedirection(statusCode);
    }
    bool isStatusCodeClientError() const {
        return HttpStatusCode_isClientError(statusCode);
    }
    bool isStatusCodeServerError() const {
        return HttpStatusCode_isServerError(statusCode);
    }
    bool isStatusCodeCustomError() const {
        return HttpStatusCode_isCustomError(statusCode);
    }
    bool isStatusCodeError() const {
        return HttpStatusCode_isError(statusCode);
    }

    QUrl replyUrl;
    HttpStatusCode statusCode;
};

QDebug operator<<(QDebug dbg, const WebResponse& arg);

struct CustomWebResponse : public WebResponse {
  public:
    static void registerMetaType();

    CustomWebResponse() = default;
    CustomWebResponse(
            WebResponse response,
            QByteArray content)
            : WebResponse(std::move(response)),
              content(std::move(content)) {
    }
    CustomWebResponse(const CustomWebResponse&) = default;
    CustomWebResponse(CustomWebResponse&&) = default;
    ~CustomWebResponse() override = default;

    CustomWebResponse& operator=(const CustomWebResponse&) = default;
    CustomWebResponse& operator=(CustomWebResponse&&) = default;

    QByteArray content;
};

QDebug operator<<(QDebug dbg, const CustomWebResponse& arg);

/// A transient task for performing a single HTTP network request
/// asynchronously.
class WebTask : public NetworkTask {
    Q_OBJECT

  public:
    explicit WebTask(
            QNetworkAccessManager* networkAccessManager,
            QObject* parent = nullptr);
    ~WebTask() override = default;

  public slots:
    void slotStart(
            int timeoutMillis) override;
    void slotAbort() override;

  private slots:
    void slotNetworkReplyFinished();

  protected:
    void timerEvent(QTimerEvent* event) final;

    enum class State {
        Idle,
        Pending,
        Aborting,
        Aborted,
        TimedOut,
        Failed,
        Finished,
    };
    State state() const {
        return m_state;
    }

    /// Handle the abort and ensure that the task eventually
    /// gets deleted. The default implementation logs a warning
    /// and deletes the task.
    virtual void onNetworkError(
            QUrl&& requestUrl,
            QNetworkReply::NetworkError errorCode,
            QString&& errorString,
            QByteArray&& errorContent);

  private:
    QUrl abortPendingNetworkReply();

    /// Try to compose and send the actual network request.
    /// Return nullptr on failure.
    virtual QNetworkReply* doStartNetworkRequest(
            QNetworkAccessManager* networkAccessManager,
            int parentTimeoutMillis) = 0;

    /// Optional: Do something after aborted.
    virtual void doNetworkReplyAborted(
            QNetworkReply* abortedNetworkReply) {
        Q_UNUSED(abortedNetworkReply);
    }

    /// Handle network response.
    virtual void doNetworkReplyFinished(
            QNetworkReply* finishedNetworkReply,
            HttpStatusCode statusCode) = 0;

    /// All member variables must only be accessed from
    /// the event loop thread!!

    int m_timeoutTimerId;
    State m_state;

    SafeQPointer<QNetworkReply> m_pendingNetworkReplyWeakPtr;
};

} // namespace network

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::network::WebResponse);

Q_DECLARE_METATYPE(mixxx::network::CustomWebResponse);
