#pragma once

#include <QNetworkRequest>
#include <QObject>
#include <QUrl>

class NetworkRequest : public QObject {
    Q_OBJECT
  public:
    explicit NetworkRequest(QUrl url)
            : m_url(std::move(url)) {
    }
    virtual void setRawHeader(const QByteArray& header, const QByteArray& value) = 0;
    virtual QList<QByteArray> rawHeaderList() const = 0;

  protected:
    QUrl m_url;
};

class QtNetworkRequest : public NetworkRequest {
    Q_OBJECT
  public:
    explicit QtNetworkRequest(QUrl url)
            : NetworkRequest(std::move(url)) {
    }
    void setRawHeader(const QByteArray& header, const QByteArray& value) override;

    QList<QByteArray> rawHeaderList() const override;

  private:
    QNetworkRequest m_request;
};
