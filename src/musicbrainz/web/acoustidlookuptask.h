#pragma once

#include <QString>
#include <QList>
#include <QUuid>

#include "network/jsonwebtask.h"

namespace mixxx {

class AcoustIdLookupTask : public network::JsonWebTask {
    Q_OBJECT

  public:
    AcoustIdLookupTask(
            QNetworkAccessManager* networkAccessManager,
            const QString& fingerprint,
            int duration,
            QObject* parent = nullptr);
    ~AcoustIdLookupTask() override = default;

  signals:
    void succeeded(
            const QList<QUuid>& recordingIds);

  protected:
    QNetworkReply* sendNetworkRequest(
            QNetworkAccessManager* networkAccessManager,
            network::HttpRequestMethod method,
            const QUrl& url,
            const QJsonDocument& content) override;

  private:
    void onFinished(
            const network::JsonWebResponse& response) override;

    void emitSucceeded(
            const QList<QUuid>& recordingIds);

    const QUrlQuery m_urlQuery;
};

} // namespace mixxx
