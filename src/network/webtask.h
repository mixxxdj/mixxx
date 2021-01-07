#pragma once

#include <QMimeType>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QUrl>

#include "network/httpstatuscode.h"
#include "network/networktask.h"
#include "util/optional.h"
#include "util/performancetimer.h"

namespace mixxx {

namespace network {

class WebResponse final {
  public:
    static void registerMetaType();

    WebResponse()
            : m_statusCode(kHttpStatusCodeInvalid) {
    }
    explicit WebResponse(
            QUrl replyUrl,
            HttpStatusCode statusCode = kHttpStatusCodeInvalid)
            : m_replyUrl(std::move(replyUrl)),
              m_statusCode(statusCode) {
    }
    WebResponse(const WebResponse&) = default;
    WebResponse(WebResponse&&) = default;

    WebResponse& operator=(const WebResponse&) = default;
    WebResponse& operator=(WebResponse&&) = default;

    bool isStatusCodeSuccess() const {
        return HttpStatusCode_isSuccess(m_statusCode);
    }

    HttpStatusCode statusCode() const {
        return m_statusCode;
    }

    const QUrl& replyUrl() const {
        return m_replyUrl;
    }

    friend QDebug operator<<(QDebug dbg, const WebResponse& arg);

  private:
    QUrl m_replyUrl;
    HttpStatusCode m_statusCode;
};

class CustomWebResponse final {
  public:
    static void registerMetaType();

    CustomWebResponse() = default;
    CustomWebResponse(
            WebResponse&& response,
            QMimeType&& contentType,
            QByteArray&& contentBytes)
            : m_response(std::move(response)),
              m_contentType(std::move(contentType)),
              m_contentBytes(std::move(contentBytes)) {
    }
    CustomWebResponse(const CustomWebResponse&) = default;
    CustomWebResponse(CustomWebResponse&&) = default;

    CustomWebResponse& operator=(const CustomWebResponse&) = default;
    CustomWebResponse& operator=(CustomWebResponse&&) = default;

    bool isStatusCodeSuccess() const {
        return m_response.isStatusCodeSuccess();
    }

    HttpStatusCode statusCode() const {
        return m_response.statusCode();
    }

    const QUrl& replyUrl() const {
        return m_response.replyUrl();
    }

    const QMimeType& contentType() const {
        return m_contentType;
    }

    const QByteArray& contentBytes() const {
        return m_contentBytes;
    }

    friend QDebug operator<<(QDebug dbg, const CustomWebResponse& arg);

  private:
    WebResponse m_response;
    QMimeType m_contentType;
    QByteArray m_contentBytes;
};

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
        // Initial state
        Idle,
        // Pending state
        Pending,
        // Terminal states
        Aborted,
        TimedOut,
        Failed,
        Finished,
    };

    State state() const {
        return m_state;
    }

    bool hasTerminated() const {
        return state() == State::Aborted ||
                state() == State::TimedOut ||
                state() == State::Failed ||
                state() == State::Finished;
    }

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

    /// Handle the abort and ensure that the task eventually
    /// gets deleted. The default implementation logs a warning
    /// and deletes the task.
    void onNetworkError(
            QUrl&& requestUrl,
            QNetworkReply::NetworkError errorCode,
            QString&& errorString,
            QByteArray&& errorContent);

    /// All member variables must only be accessed from
    /// the event loop thread!!

    State m_state;

    PerformanceTimer m_timer;

    int m_timeoutTimerId;

    SafeQPointer<QNetworkReply> m_pendingNetworkReplyWeakPtr;
};

} // namespace network

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::network::WebResponse);

Q_DECLARE_METATYPE(mixxx::network::CustomWebResponse);
