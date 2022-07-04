/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPCONNECTIONHANDLER_H
#define HTTPCONNECTIONHANDLER_H

#ifndef QT_NO_SSL
   #include <QSslConfiguration>
#endif
#include <QTcpSocket>
#include <QSettings>
#include <QTimer>
#include <QThread>
#include "httpglobal.h"
#include "httprequest.h"
#include "httprequesthandler.h"

namespace stefanfrings {

/** Alias type definition, for compatibility to different Qt versions */
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    typedef qintptr tSocketDescriptor;
#else
    typedef int tSocketDescriptor;
#endif

/** Alias for QSslConfiguration if OpenSSL is not supported */
#ifdef QT_NO_SSL
  #define QSslConfiguration QObject
#endif

/**
  The connection handler accepts incoming connections and dispatches incoming requests to to a
  request mapper. Since HTTP clients can send multiple requests before waiting for the response,
  the incoming requests are queued and processed one after the other.
  <p>
  Example for the required configuration settings:
  <code><pre>
  readTimeout=60000
  maxRequestSize=16000
  maxMultiPartSize=1000000
  </pre></code>
  <p>
  The readTimeout value defines the maximum time to wait for a complete HTTP request.
  <p>
  MaxRequestSize is the maximum size of a HTTP request. In case of
  multipart/form-data requests (also known as file-upload), the maximum
  size of the body must not exceed maxMultiPartSize.
*/
class DECLSPEC HttpConnectionHandler : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(HttpConnectionHandler)

public:

    /**
      Constructor.
      @param settings Configuration settings of the HTTP webserver
      @param requestHandler Handler that will process each incoming HTTP request
      @param sslConfiguration SSL (HTTPS) will be used if not NULL
    */
    HttpConnectionHandler(const QSettings* settings, HttpRequestHandler* requestHandler,
                          const QSslConfiguration* sslConfiguration=nullptr);

    /** Destructor */
    virtual ~HttpConnectionHandler();

    /** Returns true, if this handler is in use. */
    bool isBusy();

    /** Mark this handler as busy */
    void setBusy();

private:

    /** Configuration settings */
    const QSettings* settings;

    /** TCP socket of the current connection  */
    QTcpSocket* socket;

    /** The thread that processes events of this connection */
    QThread* thread;

    /** Time for read timeout detection */
    QTimer readTimer;

    /** Storage for the current incoming HTTP request */
    HttpRequest* currentRequest;

    /** Dispatches received requests to services */
    HttpRequestHandler* requestHandler;

    /** This shows the busy-state from a very early time */
    bool busy;

    /** Configuration for SSL */
    const QSslConfiguration* sslConfiguration;

    /**  Create SSL or TCP socket */
    void createSocket();

public slots:

    /**
      Received from from the listener, when the handler shall start processing a new connection.
      @param socketDescriptor references the accepted connection.
    */
    void handleConnection(const tSocketDescriptor socketDescriptor);

private slots:

    /** Received from the socket when a read-timeout occured */
    void readTimeout();

    /** Received from the socket when incoming data can be read */
    void read();

    /** Received from the socket when a connection has been closed */
    void disconnected();

    /** Cleanup after the thread is closed */
    void thread_done();
};

} // end of namespace

#endif // HTTPCONNECTIONHANDLER_H
