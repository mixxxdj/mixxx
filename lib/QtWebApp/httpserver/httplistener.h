/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPLISTENER_H
#define HTTPLISTENER_H

#include <QTcpServer>
#include <QSettings>
#include <QBasicTimer>
#include "httpglobal.h"
#include "httpconnectionhandler.h"
#include "httpconnectionhandlerpool.h"
#include "httprequesthandler.h"

namespace stefanfrings {

/**
  Listens for incoming TCP connections and and passes all incoming HTTP requests to your implementation of HttpRequestHandler,
  which processes the request and generates the response (usually a HTML document).
  <p>
  Example for the required settings in the config file:
  <code><pre>
  ;host=192.168.0.100
  port=8080

  readTimeout=60000
  maxRequestSize=16000
  maxMultiPartSize=1000000

  minThreads=1
  maxThreads=10
  cleanupInterval=1000

  ;sslKeyFile=ssl/server.key
  ;sslCertFile=ssl/server.crt
  ;caCertFile=ssl/ca.crt
  ;verifyPeer=false
  </pre></code>
  The optional host parameter binds the listener to a specific network interface,
  otherwise the server accepts connections from any network interface on the given port.
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
  @see HttpConnectionHandlerPool for description of the optional ssl settings
*/

class DECLSPEC HttpListener : public QTcpServer {
    Q_OBJECT
    Q_DISABLE_COPY(HttpListener)
public:

    /**
      Constructor.
      Creates a connection pool and starts listening on the configured host and port.
      @param settings Configuration settings, usually stored in an INI file. Must not be 0.
      Settings are read from the current group, so the caller must have called settings->beginGroup().
      Because the group must not change during runtime, it is recommended to provide a
      separate QSettings instance that is not used by other parts of the program.
      The HttpListener does not take over ownership of the QSettings instance, so the
      caller should destroy it during shutdown.
      @param requestHandler Processes each received HTTP request, usually by dispatching to controller classes.
      @param parent Parent object.
      @warning Ensure to close or delete the listener before deleting the request handler.
    */
    HttpListener(const QSettings* settings, HttpRequestHandler* requestHandler, QObject* parent=nullptr);

    /** Destructor */
    virtual ~HttpListener();

    /**
      Restart listeing after close().
    */
    void listen();

    /**
     Closes the listener, waits until all pending requests are processed,
     then closes the connection pool.
    */
    void close();

protected:

    /** Serves new incoming connection requests */
    void incomingConnection(tSocketDescriptor socketDescriptor);

private:

    /** Configuration settings for the HTTP server */
    const QSettings* settings;

    /** Point to the reuqest handler which processes all HTTP requests */
    HttpRequestHandler* requestHandler;

    /** Pool of connection handlers */
    HttpConnectionHandlerPool* pool;

signals:

    /**
      Sent to the connection handler to process a new incoming connection.
      @param socketDescriptor references the accepted connection.
    */

    void handleConnection(tSocketDescriptor socketDescriptor);

};

} // end of namespace

#endif // HTTPLISTENER_H
