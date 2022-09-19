#pragma once

#include <QFutureWatcher>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

#include "lastfm/lastfmgettracksimilartask.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class SuggestionFetcher : public QObject {
    Q_OBJECT

    // Fetcher of suggestion feature (can be extended with different services):
    //   1. Last.fm -> Can use different endpoints of Last.fm.

  public:
    explicit SuggestionFetcher(
            QObject* parent = nullptr);
    ~SuggestionFetcher() override = default;

    void startFetch(
            TrackPointer pTrack);

    void transmitButtonLabel(TrackPointer pTrack);

  public slots:
    void cancel();

  signals:
    void fetchSuggestionProgress(
            const QString& message);
    void suggestionResults(const QList<QMap<QString, QString>>& suggestions);
    void networkError(
            int httpStatus,
            const QString& app,
            const QString& message,
            int code);
    void changeButtonLabel(const QString& buttonLabel);

  private slots:
    void slotLastfmGetTrackSimilarTaskSucceeded(const QList<QMap<QString, QString>>& suggestions);
    void slotLastfmGetTrackSimilarTaskFailed(const mixxx::network::JsonWebResponse& response);
    void slotLastfmGetTrackSimilarTaskAborted();
    void slotLastfmGetTrackSimilarTaskNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);

  private:
    bool onLastfmGetTrackSimilarTaskTerminated();

    QNetworkAccessManager m_network;

    parented_ptr<mixxx::LastfmGetTrackSimilarTask> m_pLastfmGetTrackSimilarTask;

    TrackPointer m_pTrack;
};
