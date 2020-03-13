#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPair>
#include <QPointer>
#include <QUrl>

#include "network/httpstatuscode.h"

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

    QUrl replyUrl;
    HttpStatusCode statusCode;
    QByteArray content;
};

class WebTask : public QObject {
    Q_OBJECT

  public:
    explicit WebTask(
            QNetworkAccessManager* networkAccessManager,
            QObject* parent = nullptr);
    ~WebTask() override;

    // timeoutMillis <= 0: No timeout (unlimited)
    // timeoutMillis > 0: Implicitly aborted after timeout expired
    void invokeStart(
            int timeoutMillis = 0);

    // Cancel a pending request.
    void invokeAbort();

    // Abort the pending request while suppressing any signals
    // and mark the task for deletion.
    void deleteBeforeFinished();

    // Disconnect from all signals after receiving a reply
    // and mark the task for deletion.
    void deleteAfterFinished();

  public slots:
    void slotStart(
            int timeoutMillis);
    void slotAbort();

  signals:
    // The receiver is responsible for deleting the task in the
    // corresponding slot handler!! Otherwise the task will remain
    // in memory as a dysfunctional zombie until its parent object
    // is finally deleted. If no receiver is connected the task
    // will be deleted implicitly.
    void aborted();
    void networkError(
            QUrl requestUrl,
            QNetworkReply::NetworkError errorCode,
            QString errorString,
            QByteArray errorContent);

  protected:
    void timerEvent(QTimerEvent* event) override;

    // Handle an aborted request and ensure that the task eventually
    // gets deleted. The default implementation simply deletes the
    // task.
    virtual void onAborted();

    // Handle the abort and ensure that the task eventually
    // gets deleted. The default implementation logs a warning
    // and deletes the task.
    virtual void onNetworkError(
            QUrl requestUrl,
            QNetworkReply::NetworkError errorCode,
            QString errorString,
            QByteArray errorContent);

    QPair<QNetworkReply*, HttpStatusCode> receiveNetworkReply();

  private:
    virtual bool doStart(
            QNetworkAccessManager* networkAccessManager,
            int parentTimeoutMillis) = 0;
    virtual void doAbort() = 0;

    // All member variables must only be accessed from
    // the event loop thread!!
    QPointer<QNetworkAccessManager> m_networkAccessManager;
    int m_timeoutTimerId;
    bool m_abort;
};

} // namespace network

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::network::WebResponse);

Q_DECLARE_METATYPE(mixxx::network::CustomWebResponse);
