#pragma once

#include <QUuid>

#include "network/webtask.h"

namespace mixxx {

class CoverArtArchiveImageTask : public network::WebTask {
    Q_OBJECT

  public:
    CoverArtArchiveImageTask(
            QNetworkAccessManager* pNetworkAccessManager,
            const QString& coverArtLink,
            const QUuid& albumReleaseId,
            QObject* pParent = nullptr);
    ~CoverArtArchiveImageTask() override = default;

  signals:
    void succeeded(const QUuid& albumReleaseId,
            const QByteArray& coverArtImageBytes);

    void failed(
            const mixxx::network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);

  private:
    QNetworkReply* doStartNetworkRequest(
            QNetworkAccessManager* pNetworkAccessManager,
            int parentTimeoutMillis) override;

    void doNetworkReplyFinished(
            QNetworkReply* pFinishedNetworkReply,
            network::HttpStatusCode statusCode) override;

    void emitSucceeded(const QByteArray& coverArtImageBytes);

    void emitFailed(
            const network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);

    const QString m_coverArtUrl;
    const QUuid m_albumReleaseId;

    QByteArray coverArtImageBytes;
};

} // namespace mixxx
