#pragma once

#include <QMetaMethod>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#include "util/assert.h"
#include "util/qt.h"

namespace mixxx {

namespace network {

/// A transient task for performing a single generic network request
/// asynchronously.
///
/// The results are transmitted by emitting signals. At least one
/// of the signal receivers is responsible for destroying the task
/// by invoking QObject::deleteLater(). If no receiver is connected
/// at the time the finalization signal is emitted then the task
/// will destroy itself.
class NetworkTask : public QObject {
    Q_OBJECT

  protected:
    explicit NetworkTask(
            QNetworkAccessManager* networkAccessManager,
            QObject* parent = nullptr);
    virtual ~NetworkTask();

    /// Cancel the task by aborting the pending network request.
    ///
    /// This function is NOT thread-safe and must only be called from
    /// the event loop thread.
    void abort();

    // Required to allow derived classes to invoke abort()
    // on other instances of this class.
    void static abortThis(NetworkTask* pThis) {
        DEBUG_ASSERT(pThis);
        pThis->abort();
    }

    template<typename S>
    bool isSignalFuncConnected(
            S signalFunc) {
        const QMetaMethod signal = QMetaMethod::fromSignal(signalFunc);
        return isSignalConnected(signal);
    }

    /// Send an aborted signal with the optional request URL if available.
    void emitAborted(
            const QUrl& requestUrl = QUrl{});

    /// All member variables must only be accessed from
    /// the event loop thread!!
    const SafeQPointer<QNetworkAccessManager> m_networkAccessManagerWeakPtr;

  public:
    static constexpr int kNoTimeout = 0;

    /// Start a new task by sending a network request.
    ///
    /// timeoutMillis <= 0: No timeout (unlimited)
    /// timeoutMillis > 0: Implicitly aborted after timeout expired
    ///
    /// This function is thread-safe and could be invoked from any thread.
    void invokeStart(
            int timeoutMillis = kNoTimeout);

    /// Cancel the task by aborting the pending network request.
    ///
    /// This function is thread-safe and could be invoked from any thread.
    void invokeAbort();

  public slots:
    virtual void slotStart(
            int timeoutMillis) = 0;
    virtual void slotAbort() = 0;

  signals:
    /// The receiver is responsible for deleting the task in the
    /// corresponding slot handler!! Otherwise the task will remain
    /// in memory as a dysfunctional zombie until its parent object
    /// is finally deleted. If no receiver is connected the task
    /// will be deleted implicitly.

    /// Client-side abort
    void aborted(
            const QUrl& requestUrl);
};

} // namespace network

} // namespace mixxx
