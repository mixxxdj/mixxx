#pragma once

#include <QList>
#include <QString>
#include <QUuid>

#include "network/jsonwebtask.h"

namespace mixxx {

class CoverArtArchiveLinksTask : public network::JsonWebTask {
    Q_OBJECT

  public:
    CoverArtArchiveLinksTask(
            QNetworkAccessManager* pNetworkAccessManager,
            const QUuid& albumReleaseId,
            QObject* pParent = nullptr);

    ~CoverArtArchiveLinksTask() override = default;

  signals:
    void succeeded(const QUuid& albumReleaseId,
            const QList<QString>& allUrls);

  private:
    QNetworkReply* sendNetworkRequest(
            QNetworkAccessManager* pNetworkAccessManager,
            network::HttpRequestMethod method,
            const QUrl& url,
            const QJsonDocument& content) override;

    void onFinished(
            const network::JsonWebResponse& response) override;

    void emitSucceeded(const QList<QString>& allUrls);

    QUuid m_albumReleaseId;

    QList<QString> m_allThumbnailUrls;
};

} // namespace mixxx
