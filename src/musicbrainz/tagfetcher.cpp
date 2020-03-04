#include "musicbrainz/tagfetcher.h"

#include <QFuture>
#include <QtConcurrentRun>

#include "musicbrainz/chromaprinter.h"

namespace {

constexpr int kAcoustIdTimeoutMillis = 5000; // msec

constexpr int kMusicBrainzTimeoutMillis = 5000; // msec

mixxx::musicbrainz::TrackRelease trackReleaseFromTrack(
        const Track& track) {
    mixxx::TrackMetadata trackMetadata;
    track.readTrackMetadata(&trackMetadata);
    mixxx::musicbrainz::TrackRelease trackRelease;
    trackRelease.title = trackMetadata.getTrackInfo().getTitle();
    trackRelease.artist = trackMetadata.getTrackInfo().getArtist();
    trackRelease.trackNumber = trackMetadata.getTrackInfo().getTrackNumber();
    trackRelease.trackTotal = trackMetadata.getTrackInfo().getTrackTotal();
    trackRelease.date = trackMetadata.getTrackInfo().getYear();
    trackRelease.albumTitle = trackMetadata.getAlbumInfo().getTitle();
    trackRelease.albumArtist = trackMetadata.getAlbumInfo().getArtist();
#if defined(__EXTRA_METADATA__)
    trackRelease.discNumber = trackMetadata.getTrackInfo().getDiscNumber();
    trackRelease.discTotal = trackMetadata.getTrackInfo().getDiscTotal();
    trackRelease.artistId = trackMetadata.getTrackInfo().getMusicBrainzArtistId();
    trackRelease.recordingId = trackMetadata.getTrackInfo().getMusicBrainzRecordingId();
    trackRelease.trackReleaseId = trackMetadata.getTrackInfo().getMusicBrainzReleaseId();
    trackRelease.albumArtistId = trackMetadata.getAlbumInfo().getMusicBrainzArtistId();
    trackRelease.albumReleaseId = trackMetadata.getAlbumInfo().getMusicBrainzReleaseId();
    trackRelease.releaseGroupId = trackMetadata.getAlbumInfo().getMusicBrainzReleaseGroupId();
#endif // __EXTRA_METADATA__
    trackRelease.duration = trackMetadata.getDuration();
    return trackRelease;
}

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
        emit resultAvailable(
                trackReleaseFromTrack(*m_pTrack),
                QList<mixxx::musicbrainz::TrackRelease>());
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
        emit resultAvailable(
                trackReleaseFromTrack(*m_pTrack),
                QList<mixxx::musicbrainz::TrackRelease>());
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
        QList<mixxx::musicbrainz::TrackRelease> guessedTrackReleases) {
    auto pOriginalTrack = std::move(m_pTrack);
    abortMusicBrainzTask();
    if (!pOriginalTrack) {
        // aborted
        return;
    }
    const mixxx::musicbrainz::TrackRelease originalTrackRelease =
            trackReleaseFromTrack(*pOriginalTrack);
    emit resultAvailable(
            std::move(originalTrackRelease),
            std::move(guessedTrackReleases));
}
