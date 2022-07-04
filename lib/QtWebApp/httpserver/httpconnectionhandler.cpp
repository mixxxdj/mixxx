/**
  @file
  @author Stefan Frings
*/

#include "httpconnectionhandler.h"
#include "httpresponse.h"

using namespace stefanfrings;

HttpConnectionHandler::HttpConnectionHandler(const QSettings *settings, HttpRequestHandler *requestHandler, const QSslConfiguration* sslConfiguration)
    : QObject()
{
    Q_ASSERT(settings!=nullptr);
    Q_ASSERT(requestHandler!=nullptr);
    this->settings=settings;
    this->requestHandler=requestHandler;
    this->sslConfiguration=sslConfiguration;
    currentRequest=nullptr;
    busy=false;

    // execute signals in a new thread
    thread = new QThread();
    thread->start();
    qDebug("HttpConnectionHandler (%p): thread started", static_cast<void*>(this));
    moveToThread(thread);
    readTimer.moveToThread(thread);
    readTimer.setSingleShot(true);

    // Create TCP or SSL socket
    createSocket();
    socket->moveToThread(thread);

    // Connect signals
    connect(socket, SIGNAL(readyRead()), SLOT(read()));
    connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&readTimer, SIGNAL(timeout()), SLOT(readTimeout()));
    connect(thread, SIGNAL(finished()), this, SLOT(thread_done()));

    qDebug("HttpConnectionHandler (%p): constructed", static_cast<void*>(this));
}


void HttpConnectionHandler::thread_done()
{
    readTimer.stop();
    socket->close();
    delete socket;
    qDebug("HttpConnectionHandler (%p): thread stopped", static_cast<void*>(this));
}


HttpConnectionHandler::~HttpConnectionHandler()
{
    thread->quit();
    thread->wait();
    thread->deleteLater();
    qDebug("HttpConnectionHandler (%p): destroyed", static_cast<void*>(this));
}


void HttpConnectionHandler::createSocket()
{
    // If SSL is supported and configured, then create an instance of QSslSocket
    #ifndef QT_NO_SSL
        if (sslConfiguration)
        {
            QSslSocket* sslSocket=new QSslSocket();
            sslSocket->setSslConfiguration(*sslConfiguration);
            socket=sslSocket;
            qDebug("HttpConnectionHandler (%p): SSL is enabled", static_cast<void*>(this));
            return;
        }
    #endif
    // else create an instance of QTcpSocket
    socket=new QTcpSocket();
}


void HttpConnectionHandler::handleConnection(tSocketDescriptor socketDescriptor)
{
    qDebug("HttpConnectionHandler (%p): handle new connection", static_cast<void*>(this));
    busy = true;
    Q_ASSERT(socket->isOpen()==false); // if not, then the handler is already busy

    //UGLY workaround - we need to clear writebuffer before reusing this socket
    //https://bugreports.qt-project.org/browse/QTBUG-28914
    socket->connectToHost("",0);
    socket->abort();

    if (!socket->setSocketDescriptor(socketDescriptor))
    {
        qCritical("HttpConnectionHandler (%p): cannot initialize socket: %s",
                  static_cast<void*>(this),qPrintable(socket->errorString()));
        return;
    }

    #ifndef QT_NO_SSL
        // Switch on encryption, if SSL is configured
        if (sslConfiguration)
        {
            qDebug("HttpConnectionHandler (%p): Starting encryption", static_cast<void*>(this));
            (static_cast<QSslSocket*>(socket))->startServerEncryption();
        }
    #endif

    // Start timer for read timeout
    int readTimeout=settings->value("readTimeout",10000).toInt();
    readTimer.start(readTimeout);
    // delete previous request
    delete currentRequest;
    currentRequest=nullptr;
}


bool HttpConnectionHandler::isBusy()
{
    return busy;
}

void HttpConnectionHandler::setBusy()
{
    this->busy = true;
}


void HttpConnectionHandler::readTimeout()
{
    qDebug("HttpConnectionHandler (%p): read timeout occured",static_cast<void*>(this));

    //Commented out because QWebView cannot handle this.
    //socket->write("HTTP/1.1 408 request timeout\r\nConnection: close\r\n\r\n408 request timeout\r\n");

    while(socket->bytesToWrite()) socket->waitForBytesWritten();
    socket->disconnectFromHost();
    delete currentRequest;
    currentRequest=nullptr;
}


void HttpConnectionHandler::disconnected()
{
    qDebug("HttpConnectionHandler (%p): disconnected", static_cast<void*>(this));
    socket->close();
    readTimer.stop();
    busy = false;
}

void HttpConnectionHandler::read()
{
    // The loop adds support for HTTP pipelinig
    while (socket->bytesAvailable())
    {
        #ifdef SUPERVERBOSE
            qDebug("HttpConnectionHandler (%p): read input",static_cast<void*>(this));
        #endif

        // Create new HttpRequest object if necessary
        if (!currentRequest)
        {
            currentRequest=new HttpRequest(settings);
        }

        // Collect data for the request object
        while (socket->bytesAvailable() && currentRequest->getStatus()!=HttpRequest::complete && currentRequest->getStatus()!=HttpRequest::abort)
        {
            currentRequest->readFromSocket(socket);
            if (currentRequest->getStatus()==HttpRequest::waitForBody)
            {
                // Restart timer for read timeout, otherwise it would
                // expire during large file uploads.
                int readTimeout=settings->value("readTimeout",10000).toInt();
                readTimer.start(readTimeout);
            }
        }

        // If the request is aborted, return error message and close the connection
        if (currentRequest->getStatus()==HttpRequest::abort)
        {
            socket->write("HTTP/1.1 413 entity too large\r\nConnection: close\r\n\r\n413 Entity too large\r\n");
            while(socket->bytesToWrite()) socket->waitForBytesWritten();
            socket->disconnectFromHost();
            delete currentRequest;
            currentRequest=nullptr;
            return;
        }

        // If the request is complete, let the request mapper dispatch it
        if (currentRequest->getStatus()==HttpRequest::complete)
        {
            readTimer.stop();
            qDebug("HttpConnectionHandler (%p): received request",static_cast<void*>(this));

            // Copy the Connection:close header to the response
            HttpResponse response(socket);
            bool closeConnection=QString::compare(currentRequest->getHeader("Connection"),"close",Qt::CaseInsensitive)==0;
            if (closeConnection)
            {
                response.setHeader("Connection","close");
            }

            // In case of HTTP 1.0 protocol add the Connection:close header.
            // This ensures that the HttpResponse does not activate chunked mode, which is not spported by HTTP 1.0.
            else
            {
                bool http1_0=QString::compare(currentRequest->getVersion(),"HTTP/1.0",Qt::CaseInsensitive)==0;
                if (http1_0)
                {
                    closeConnection=true;
                    response.setHeader("Connection","close");
                }
            }

            // Call the request mapper
            try
            {
                requestHandler->service(*currentRequest, response);
            }
            catch (...)
            {
                qCritical("HttpConnectionHandler (%p): An uncatched exception occured in the request handler",
                          static_cast<void*>(this));
            }

            // Finalize sending the response if not already done
            if (!response.hasSentLastPart())
            {
                response.write(QByteArray(),true);
            }

            qDebug("HttpConnectionHandler (%p): finished request",static_cast<void*>(this));

            // Find out whether the connection must be closed
            if (!closeConnection)
            {
                // Maybe the request handler or mapper added a Connection:close header in the meantime
                bool closeResponse=QString::compare(response.getHeaders().value("Connection"),"close",Qt::CaseInsensitive)==0;
                if (closeResponse==true)
                {
                    closeConnection=true;
                }
                else
                {
                    // If we have no Content-Length header and did not use chunked mode, then we have to close the
                    // connection to tell the HTTP client that the end of the response has been reached.
                    bool hasContentLength=response.getHeaders().contains("Content-Length");
                    if (!hasContentLength)
                    {
                        bool hasChunkedMode=QString::compare(response.getHeaders().value("Transfer-Encoding"),"chunked",Qt::CaseInsensitive)==0;
                        if (!hasChunkedMode)
                        {
                            closeConnection=true;
                        }
                    }
                }
            }

            // Close the connection or prepare for the next request on the same connection.
            if (closeConnection)
            {
                while(socket->bytesToWrite()) socket->waitForBytesWritten();
                socket->disconnectFromHost();
            }
            else
            {
                // Start timer for next request
                int readTimeout=settings->value("readTimeout",10000).toInt();
                readTimer.start(readTimeout);
            }
            delete currentRequest;
            currentRequest=nullptr;
        }
    }
}
