#pragma once

#include <QList>
#include <QString>
#include <QUrlQuery>
#include <QUuid>

#include "network/webtask.h"

namespace mixxx {

class LastfmGetTrackSimilarTask : public network::WebTask {
    Q_OBJECT

  public:
    LastfmGetTrackSimilarTask(
            QNetworkAccessManager* networkAccessManager,
            const QString& artist,
            const QString& track,
            QObject* parent = nullptr);
    ~LastfmGetTrackSimilarTask() override = default;

  signals:
    void succeeded(const QByteArray& response);

    void failed(
            const network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);

  private:
    QNetworkReply* doStartNetworkRequest(
            QNetworkAccessManager* networkAccessManager,
            int parentTimeoutMillis) override;
    void doNetworkReplyFinished(
            QNetworkReply* finishedNetworkReply,
            network::HttpStatusCode statusCode) override;

    void emitSucceeded(
            const QByteArray& response);
    void emitFailed(
            const network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);

    const QUrlQuery m_urlQuery;
};

} // namespace mixxx
