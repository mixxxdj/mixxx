#include "musicbrainz/tagfetcher.h"

#include <QFuture>
#include <QtConcurrentRun>

#include "musicbrainz/chromaprinter.h"

namespace {

constexpr int kAcoustIdTimeoutMillis = 5000; // msec

constexpr int kMusicBrainzTimeoutMillis = 5000; // msec

} // anonymous namespace

TagFetcher::TagFetcher(QObject* parent)
        : QObject(parent),
          m_fingerprintWatcher(this) {
}

void TagFetcher::startFetch(
        TrackPointer pTrack) {
    cancel();

    m_pTrack = pTrack;

    emit fetchProgress(tr("Fingerprinting track"));
    const auto fingerprintTask = QtConcurrent::run([this, pTrack] {
        return ChromaPrinter(this).getFingerprint(pTrack);
    });
    m_fingerprintWatcher.setFuture(fingerprintTask);
    connect(
            &m_fingerprintWatcher,
            &QFutureWatcher<QString>::finished,
            this,
            &TagFetcher::slotFingerprintReady);
}

void TagFetcher::abortAcoustIdTask() {
    if (m_pAcoustIdTask) {
        disconnect(m_pAcoustIdTask.get());
        m_pAcoustIdTask->deleteBeforeFinished();
        m_pAcoustIdTask = nullptr;
    }
}

void TagFetcher::abortMusicBrainzTask() {
    if (m_pMusicBrainzTask) {
        disconnect(m_pMusicBrainzTask.get());
        m_pMusicBrainzTask->deleteBeforeFinished();
        m_pMusicBrainzTask = nullptr;
    }
}

void TagFetcher::cancel() {
    // qDebug()<< "Cancel tagfetching";
    m_fingerprintWatcher.cancel();
    abortAcoustIdTask();
    abortMusicBrainzTask();
    m_pTrack.reset();
}

void TagFetcher::slotFingerprintReady() {
    if (!m_pTrack ||
            !m_fingerprintWatcher.isFinished()) {
        return;
    }

    const QString fingerprint = m_fingerprintWatcher.result();
    if (fingerprint.isEmpty()) {
        emit resultAvailable(m_pTrack, QList<TrackPointer>());
        return;
    }

    abortAcoustIdTask();

    emit fetchProgress(tr("Identifying track through Acoustid"));
    DEBUG_ASSERT(!m_pAcoustIdTask);
    m_pAcoustIdTask = make_parented<mixxx::AcoustIdLookupTask>(
            &m_network,
            fingerprint,
            m_pTrack->getDurationInt(),
            this);
    connect(m_pAcoustIdTask.get(),
            &mixxx::AcoustIdLookupTask::succeeded,
            this,
            &TagFetcher::slotAcoustIdTaskSucceeded);
    connect(m_pAcoustIdTask.get(),
            &mixxx::AcoustIdLookupTask::failed,
            this,
            &TagFetcher::slotAcoustIdTaskFailed);
    connect(m_pAcoustIdTask.get(),
            &mixxx::AcoustIdLookupTask::networkError,
            this,
            &TagFetcher::slotAcoustIdTaskNetworkError);
    m_pAcoustIdTask->invokeStart(
            kAcoustIdTimeoutMillis);
}

void TagFetcher::slotAcoustIdTaskSucceeded(
        QList<QUuid> recordingIds) {
    abortAcoustIdTask();
    if (!m_pTrack) {
        return;
    }

    if (recordingIds.isEmpty()) {
        emit resultAvailable(m_pTrack, QList<TrackPointer>());
        return;
    }

    emit fetchProgress(tr("Retrieving metadata from MusicBrainz"));
    DEBUG_ASSERT(!m_pMusicBrainzTask);
    m_pMusicBrainzTask = make_parented<mixxx::MusicBrainzRecordingsTask>(
            &m_network,
            std::move(recordingIds),
            this);
    connect(m_pMusicBrainzTask.get(),
            &mixxx::MusicBrainzRecordingsTask::succeeded,
            this,
            &TagFetcher::slotMusicBrainzTaskSucceeded);
    connect(m_pMusicBrainzTask.get(),
            &mixxx::MusicBrainzRecordingsTask::failed,
            this,
            &TagFetcher::slotMusicBrainzTaskFailed);
    connect(m_pMusicBrainzTask.get(),
            &mixxx::MusicBrainzRecordingsTask::networkError,
            this,
            &TagFetcher::slotMusicBrainzTaskNetworkError);
    m_pMusicBrainzTask->invokeStart(
            kMusicBrainzTimeoutMillis);
}

void TagFetcher::slotAcoustIdTaskFailed(
        mixxx::network::JsonWebResponse response) {
    cancel();
    emit networkError(
            response.statusCode,
            "AcoustID",
            response.content.toJson(),
            -1);
}

void TagFetcher::slotAcoustIdTaskNetworkError(
        QUrl requestUrl,
        QNetworkReply::NetworkError errorCode,
        QString errorString,
        QByteArray errorContent) {
    Q_UNUSED(requestUrl);
    Q_UNUSED(errorContent);
    cancel();
    emit networkError(
            mixxx::network::kHttpStatusCodeInvalid,
            "AcoustID",
            errorString,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskNetworkError(
        QUrl requestUrl,
        QNetworkReply::NetworkError errorCode,
        QString errorString,
        QByteArray errorContent) {
    Q_UNUSED(requestUrl);
    Q_UNUSED(errorContent);
    cancel();
    emit networkError(
            mixxx::network::kHttpStatusCodeInvalid,
            "MusicBrainz",
            errorString,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskFailed(
        mixxx::network::WebResponse response,
        int errorCode,
        QString errorMessage) {
    cancel();
    emit networkError(
            response.statusCode,
            "MusicBrainz",
            errorMessage,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskSucceeded(
        QList<mixxx::musicbrainz::TrackRelease> trackReleases) {
    auto pOriginalTrack = std::move(m_pTrack);
    abortMusicBrainzTask();
    if (!pOriginalTrack) {
        return;
    }

    QList<TrackPointer> tracksGuessed;
    for (const auto& trackRelease : qAsConst(trackReleases)) {
        mixxx::TrackMetadata trackMetadata;
        trackMetadata.refTrackInfo().setTitle(trackRelease.title);
        trackMetadata.refTrackInfo().setArtist(trackRelease.artist);
        trackMetadata.refTrackInfo().setTrackNumber(trackRelease.trackNumber);
        trackMetadata.refTrackInfo().setTrackTotal(trackRelease.trackTotal);
        trackMetadata.refTrackInfo().setYear(trackRelease.date);
        trackMetadata.refAlbumInfo().setTitle(trackRelease.albumTitle);
        trackMetadata.refAlbumInfo().setArtist(trackRelease.albumArtist);
#if defined(__EXTRA_METADATA__)
        trackMetadata.refTrackInfo().setDiscNumber(trackRelease.discNumber);
        trackMetadata.refTrackInfo().setDiscTotal(trackRelease.discTotal);
        trackMetadata.refTrackInfo().setMusicBrainzArtistId(trackRelease.artistId);
        trackMetadata.refTrackInfo().setMusicBrainzRecordingId(trackRelease.recordingId);
        trackMetadata.refTrackInfo().setMusicBrainzReleaseId(trackRelease.trackReleaseId);
        trackMetadata.refAlbumInfo().setMusicBrainzArtistId(trackRelease.albumArtistId);
        trackMetadata.refAlbumInfo().setMusicBrainzReleaseId(trackRelease.albumReleaseId);
        trackMetadata.refAlbumInfo().setMusicBrainzReleaseGroupId(trackRelease.releaseGroupId);
#endif // __EXTRA_METADATA__
        TrackPointer pTrack =
                Track::newTemporary(
                        pOriginalTrack->getFileInfo(),
                        pOriginalTrack->getSecurityToken());
        pTrack->importMetadata(std::move(trackMetadata));
        pTrack->setDuration(trackRelease.duration);
        tracksGuessed << pTrack;
    }
    emit resultAvailable(
            std::move(pOriginalTrack),
            std::move(tracksGuessed));
}
