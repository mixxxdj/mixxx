
#ifndef NETWORK_H
#define NETWORK_H

#include <QAbstractNetworkCache>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NetworkAccessManager : public QNetworkAccessManager {
  Q_OBJECT

public:
  NetworkAccessManager(QObject* parent = 0);

protected:
  QNetworkReply* createRequest(Operation op, const QNetworkRequest& request,
                               QIODevice* outgoingData);
};


class NetworkTimeouts : public QObject {
  Q_OBJECT

public:
  NetworkTimeouts(int timeout_msec, QObject* parent = 0);

  void AddReply(QNetworkReply* reply);
  void SetTimeout(int msec) { timeout_msec_ = msec; }

protected:
  void timerEvent(QTimerEvent* e);

private slots:
  void ReplyFinished();

private:
  int timeout_msec_;
  QMap<QNetworkReply*, int> timers_;
};

#endif // NETWORK_H
