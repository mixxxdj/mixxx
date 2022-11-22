#pragma once

#include "network/webtask.h"

namespace mixxx {

class CoverArtArchiveImageTask : public network::WebTask {
    Q_OBJECT

  public:
    CoverArtArchiveImageTask(
            QNetworkAccessManager* pNetworkAccessManager,
            const QString& coverArtLink,
            QObject* pParent = nullptr);
    ~CoverArtArchiveImageTask() override = default;

  signals:
    void succeeded(
            const QByteArray& coverArtImageBytes);

    void failed(
            const network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);

  private:
    QNetworkReply* doStartNetworkRequest(
            QNetworkAccessManager* pNetworkAccessManager,
            int parentTimeoutMillis) override;

    void doNetworkReplyFinished(
            QNetworkReply* pFinishedNetworkReply,
            network::HttpStatusCode statusCode) override;

    QString m_coverArtUrl;

    QByteArray coverArtImageBytes;
};

} // namespace mixxx
