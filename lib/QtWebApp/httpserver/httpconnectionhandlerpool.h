#ifndef HTTPCONNECTIONHANDLERPOOL_H
#define HTTPCONNECTIONHANDLERPOOL_H

#include <QList>
#include <QTimer>
#include <QObject>
#include <QMutex>
#include "httpglobal.h"
#include "httpconnectionhandler.h"

namespace stefanfrings {

/**
  Pool of http connection handlers. The size of the pool grows and
  shrinks on demand.
  <p>
  Example for the required configuration settings:
  <code><pre>
  readTimeout=60000
  maxRequestSize=16000
  maxMultiPartSize=1000000

  minThreads=4
  maxThreads=100
  cleanupInterval=60000  
  </pre></code>
  <p>
  The readTimeout value defines the maximum time to wait for a complete HTTP request.
  <p>
  MaxRequestSize is the maximum size of a HTTP request. In case of
  multipart/form-data requests (also known as file-upload), the maximum
  size of the body must not exceed maxMultiPartSize.
  <p>
  After server start, the size of the thread pool is always 0. Threads
  are started on demand when requests come in. The cleanup timer reduces
  the number of idle threads slowly by closing one thread in each interval.
  But the configured minimum number of threads are kept running.
  <p>
  Additional settings for SSL (HTTPS):
  <code><pre>
  sslKeyFile=ssl/server.key
  sslCertFile=ssl/server.crt
  ;caCertFile=ssl/ca.crt
  verifyPeer=false
  </pre></code>
  For SSL support, you need at least a pair of OpenSSL x509 certificate and an RSA key,
  both files in PEM format. To enable verification of the peer (the calling web browser),
  you can either use the central certificate store of the operating system, or provide
  a CA certificate file in PEM format. The certificates of the peers must have been
  derived from the CA certificate.
  <p>
  Example commands to create these files:
  <code><pre>
  # Generate CA key
  openssl genrsa 2048 > ca.key

  # Generate CA certificate
  openssl req -new -x509 -nodes -days 365000 -key ca.key -out ca.crt

  # Generate a server key and certificate request
  openssl req -newkey rsa:2048 -nodes -days 365000 -keyout server.key -out server.req

  # Generate a signed server certificate
  openssl x509 -req -days 365000 -set_serial 01 -in server.req -out server.crt -CA ca.crt -CAkey ca.key

  # Generate a client key and certificate request
  openssl req -newkey rsa:2048 -nodes -days 365000 -keyout client.key -out client.req

  # Generate a signed client certificate
  openssl x509 -req -days 365000 -set_serial 01 -in client.req -out client.crt  -CA ca.crt -CAkey ca.key

  # Combine client key and certificate into one PKCS12 file
  openssl pkcs12 -export -in client.crt -inkey client.key -out client.p12 -certfile ca.crt

  # Remove temporary files
  rm *.req
  </pre></code>
  <p>
  Please note that a listener with SSL can only handle HTTPS protocol. To support both
  HTTP and HTTPS simultaneously, you need to start <b>two</b> listeners on different ports
  one with SLL and one without SSL (usually on public ports 80 and 443, or locally on 8080 and 8443).
*/

class DECLSPEC HttpConnectionHandlerPool : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(HttpConnectionHandlerPool)
public:

    /**
      Constructor.
      @param settings Configuration settings for the HTTP server. Must not be 0.
      @param requestHandler The handler that will process each received HTTP request.
    */
    HttpConnectionHandlerPool(const QSettings* settings, HttpRequestHandler *requestHandler);

    /** Destructor */
    virtual ~HttpConnectionHandlerPool();

    /** Get a free connection handler, or 0 if not available. */
    HttpConnectionHandler* getConnectionHandler();

private:

    /** Settings for this pool */
    const QSettings* settings;

    /** Will be assigned to each Connectionhandler during their creation */
    HttpRequestHandler* requestHandler;

    /** Pool of connection handlers */
    QList<HttpConnectionHandler*> pool;

    /** Timer to clean-up unused connection handler */
    QTimer cleanupTimer;

    /** Used to synchronize threads */
    QMutex mutex;

    /** The SSL configuration (certificate, key and other settings) */
    QSslConfiguration* sslConfiguration;

    /** Load SSL configuration */
    void loadSslConfig();

private slots:

    /** Received from the clean-up timer.  */
    void cleanup();

};

} // end of namespace

#endif // HTTPCONNECTIONHANDLERPOOL_H
