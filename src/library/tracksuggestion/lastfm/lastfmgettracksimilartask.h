#pragma once

#include <QList>
#include <QString>
#include <QUrlQuery>
#include <QUuid>

#include "network/jsonwebtask.h"

namespace mixxx {

class LastfmGetTrackSimilarTask : public network::JsonWebTask {
    Q_OBJECT

  public:
    LastfmGetTrackSimilarTask(
            QNetworkAccessManager* networkAccessManager,
            const QString& artist,
            const QString& track,
            QObject* parent = nullptr);
    ~LastfmGetTrackSimilarTask() override = default;

  signals:
    void succeeded(const QList<QMap<QString, QString>>& suggestions);

  protected:
    QNetworkReply* sendNetworkRequest(
            QNetworkAccessManager* networkAccessManager,
            network::HttpRequestMethod method,
            const QUrl& url,
            const QJsonDocument& content) override;

  private:
    void onFinished(
            const network::JsonWebResponse& response) override;

    void emitSucceeded(const QList<QMap<QString, QString>>& suggestions);

    const QUrlQuery m_urlQuery;
};

} // namespace mixxx
