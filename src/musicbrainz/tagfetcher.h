#pragma once

#include <QFutureWatcher>
#include <QObject>

#include "musicbrainz/web/acoustidlookuptask.h"
#include "musicbrainz/web/coverartarchiveimagetask.h"
#include "musicbrainz/web/coverartarchivelinkstask.h"
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

    // This is called from dlgTagFetcher.
    // This starts the initial task for to find the cover art links
    // 4 Possible cover art links fetched in this task.
    // This can be >1200px-1200px-500px-250px image links.
    void startFetchCoverArtLinks(const QUuid& albumReleaseId);

    // After the first task is done successfully.
    // This is called automatically.
    // This task starts to fetch the image.
    // Link provided from preference option.
    // After a success task, related label updated with cover art.
    // If user presses apply, cover art downloaded and applied to the song.
    void startFetchCoverArtImage(const QUuid& albumReleaseId,
            const QString& coverArtUrl);

  public slots:
    void cancel();

  signals:
    void resultAvailable(
            TrackPointer pTrack,
            const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases,
            const QString& whyEmptyMessage); // To explain why the result is empty
    void fetchProgress(
            const QString& message);
    void numberOfRecordingsFoundFromAcoustId(int totalNumberOfRecordings);
    void currentRecordingFetchedFromMusicBrainz();
    void networkError(
            int httpStatus,
            const QString& app,
            const QString& message,
            int code);
    void fetchedCoverUpdate(const QByteArray& coverInfo);
    void coverArtImageFetchAvailable(const QUuid& albumReleaseId,
            const QByteArray& coverArtBytes);
    void coverArtArchiveLinksAvailable(const QUuid& albumReleaseId,
            const QList<QString>& allUrls);
    void coverArtLinkNotFound();

  private slots:
    void slotFingerprintReady();

    void slotAcoustIdTaskSucceeded(
            QList<QUuid> recordingIds);
    void slotAcoustIdTaskFailed(
            const mixxx::network::JsonWebResponse& response);
    void slotAcoustIdTaskAborted();
    void slotAcoustIdTaskNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);

    void slotMusicBrainzTaskSucceeded(
            const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases);
    void slotMusicBrainzTaskFailed(
            const mixxx::network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);
    void slotMusicBrainzTaskAborted();
    void slotMusicBrainzTaskNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);

    void slotCoverArtArchiveLinksTaskSucceeded(const QUuid& albumReleaseId,
            const QList<QString>& allUrls);
    void slotCoverArtArchiveLinksTaskFailed(
            const mixxx::network::JsonWebResponse& response);
    void slotCoverArtArchiveLinksTaskAborted();
    void slotCoverArtArchiveLinksTaskNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);

    void slotCoverArtArchiveImageTaskSucceeded(const QUuid& albumReleaseId,
            const QByteArray& coverArtBytes);
    void slotCoverArtArchiveImageTaskFailed(
            const mixxx::network::WebResponse& response,
            int errorCode,
            const QString& errorMessage);
    void slotCoverArtArchiveImageTaskAborted();
    void slotCoverArtArchiveImageTaskNetworkError(
            QNetworkReply::NetworkError errorCode,
            const QString& errorString,
            const mixxx::network::WebResponseWithContent& responseWithContent);

  private:
    void terminate();

    QNetworkAccessManager m_network;

    QFutureWatcher<QString> m_fingerprintWatcher;

    parented_ptr<mixxx::AcoustIdLookupTask> m_pAcoustIdTask;

    parented_ptr<mixxx::MusicBrainzRecordingsTask> m_pMusicBrainzTask;

    parented_ptr<mixxx::CoverArtArchiveLinksTask> m_pCoverArtArchiveLinksTask;

    parented_ptr<mixxx::CoverArtArchiveImageTask> m_pCoverArtArchiveImageTask;

    TrackPointer m_pTrack;
};
