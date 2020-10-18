#pragma once

#include <QFutureWatcher>
#include <QObject>

#include "musicbrainz/web/acoustidlookuptask.h"
#include "musicbrainz/web/musicbrainzrecordingstask.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class TagFetcher : public QObject {
    Q_OBJECT

    // Implements retrieval of metadata in 3 subsequent stages:
    //   1. Chromaprint -> AcoustID fingerprint
    //   2. AcoustID -> MusicBrainz recording UUIDs
    //   3. MusicBrainz -> MusicBrainz track releases

  public:
    explicit TagFetcher(
            QObject* parent = nullptr);
    ~TagFetcher() override = default;

    void startFetch(
            TrackPointer pTrack);

  public slots:
    void cancel();

  signals:
    void resultAvailable(
            TrackPointer pTrack,
            QList<mixxx::musicbrainz::TrackRelease> guessedTrackReleases);
    void fetchProgress(
            QString message);
    void networkError(
            int httpStatus,
            QString app,
            QString message,
            int code);

  private slots:
    void slotFingerprintReady();

    void slotAcoustIdTaskSucceeded(
            QList<QUuid> recordingIds);
    void slotAcoustIdTaskFailed(
            mixxx::network::JsonWebResponse response);
    void slotAcoustIdTaskAborted();
    void slotAcoustIdTaskNetworkError(
            QUrl requestUrl,
            QNetworkReply::NetworkError errorCode,
            QString errorString,
            QByteArray errorContent);

    void slotMusicBrainzTaskSucceeded(
            QList<mixxx::musicbrainz::TrackRelease> guessedTrackReleases);
    void slotMusicBrainzTaskFailed(
            mixxx::network::WebResponse response,
            int errorCode,
            QString errorMessage);
    void slotMusicBrainzTaskAborted();
    void slotMusicBrainzTaskNetworkError(
            QUrl requestUrl,
            QNetworkReply::NetworkError errorCode,
            QString errorString,
            QByteArray errorContent);

  private:
    QNetworkAccessManager m_network;

    QFutureWatcher<QString> m_fingerprintWatcher;

    parented_ptr<mixxx::AcoustIdLookupTask> m_pAcoustIdTask;

    parented_ptr<mixxx::MusicBrainzRecordingsTask> m_pMusicBrainzTask;

    TrackPointer m_pTrack;
};
