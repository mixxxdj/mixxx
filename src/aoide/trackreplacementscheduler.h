#pragma once

#include <QList>
#include <QPointer>
#include <QQueue>
#include <QVector>

#include "aoide/json/collection.h"
#include "aoide/json/track.h"
#include "aoide/web/replacecollectedtrackstask.h"
#include "track/track.h"
#include "track/trackref.h"

namespace mixxx {

class TrackLoader;

}

namespace aoide {

class Gateway;

class TrackReplacementScheduler : public QObject {
    Q_OBJECT

  public:
    TrackReplacementScheduler(
            Gateway* gateway,
            mixxx::TrackLoader* trackLoader,
            QObject* parent = nullptr);
    ~TrackReplacementScheduler() override = default;

    void scheduleReplaceTracks(
            QString collectionUid,
            json::MediaSourceConfig collectionMediaSourceConfig,
            QList<TrackRef> trackRefs);

    void invokeCancel();

  public slots:
    void slotReplaceTracks(
            QString collectionUid,
            const QJsonValue& collectionMediaSourceConfigJson,
            const QList<TrackRef>& trackRefs);

    void slotCancel();

  signals:
    // total = queued + pending + succeeded + failed
    void progress(int queued, int pending, int succeeded, int failed);

  private slots:
    void /*TrackLoader*/ onTrackLoaded(
            TrackRef trackRef,
            TrackPointer trackPtr);

    void slotReplaceTracksNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);

    void slotReplaceTracksAborted();

    void slotReplaceTracksFailed(
            mixxx::network::JsonWebResponse response);

    void slotReplaceTracksSucceeded(
            QJsonObject result);

  private:
    void makeProgress();
    void emitProgress();

    const QPointer<Gateway> m_gateway;

    const QPointer<mixxx::TrackLoader> m_trackLoader;

    // Requests for different collections
    QQueue<std::pair<QString, QList<TrackRef>>> m_deferredRequests;

    QString m_collectionUid;
    json::MediaSourceConfig m_collectionMediaSourceConfig;

    QQueue<TrackRef> m_queuedTrackRefs;

    QVector<TrackRef> m_loadingTrackRefs;

    bool isLoading(
            const TrackRef& trackRef) const;
    bool enterLoading(
            const TrackRef& trackRef);
    bool leaveLoading(
            const TrackRef& trackRef);

    QList<json::Track> m_bufferedRequests;

    int m_pendingCounter;
    int m_succeededCounter;
    int m_failedCounter;
};

} // namespace aoide
