#pragma once

#include <QNetworkAccessManager>
#include <QObject>

class QByteArray;
class QNetworkReply;
class QNetworkRequest;
class NetworkReply;
class NetworkRequest;

/// Handles network communications
class NetworkManager : public QObject {
    Q_OBJECT
  public:
    virtual NetworkReply* post(const NetworkRequest* request, const QByteArray& data) = 0;
  signals:
    void finished(NetworkReply* reply);
};
