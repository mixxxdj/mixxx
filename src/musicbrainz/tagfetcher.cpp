#include "musicbrainz/tagfetcher.h"

#include <QFuture>
#include <QtConcurrentRun>

#include "moc_tagfetcher.cpp"
#include "musicbrainz/chromaprinter.h"
#include "track/track.h"
#include "util/thread_affinity.h"

namespace {

// Long timeout to cope with occasional server-side unresponsiveness
constexpr int kAcoustIdTimeoutMillis = 60000; // msec

// Long timeout to cope with occasional server-side unresponsiveness
constexpr int kMusicBrainzTimeoutMillis = 60000; // msec

// Long timeout to cope with occasional server-side unresponsiveness
constexpr int kCoverArtArchiveLinksTimeoutMilis = 60000; // msec

// Long timeout to cope with occasional server-side unresponsiveness
constexpr int kCoverArtArchiveImageTimeoutMilis = 60000; // msec

} // anonymous namespace

TagFetcher::TagFetcher(QObject* parent)
        : QObject(parent),
          m_fingerprintWatcher(this) {
}

void TagFetcher::startFetch(
        TrackPointer pTrack) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    terminate();

    m_pTrack = pTrack;

    emit fetchProgress(tr("Fingerprinting track"));
    const auto fingerprintTask = QtConcurrent::run([pTrack] {
        return ChromaPrinter().getFingerprint(pTrack);
    });
    m_fingerprintWatcher.setFuture(fingerprintTask);
    DEBUG_ASSERT(!m_pAcoustIdTask);
    connect(
            &m_fingerprintWatcher,
            &QFutureWatcher<QString>::finished,
            this,
            &TagFetcher::slotFingerprintReady);
}

void TagFetcher::cancel() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    m_pTrack.reset();
    m_fingerprintWatcher.disconnect(this);
    m_fingerprintWatcher.cancel();
    if (m_pAcoustIdTask) {
        m_pAcoustIdTask->invokeAbort();
        DEBUG_ASSERT(!m_pAcoustIdTask);
    }
    if (m_pMusicBrainzTask) {
        m_pMusicBrainzTask->invokeAbort();
        DEBUG_ASSERT(!m_pMusicBrainzTask);
    }
    if (m_pCoverArtArchiveLinksTask) {
        m_pCoverArtArchiveLinksTask->invokeAbort();
        DEBUG_ASSERT(!m_pCoverArtArchiveLinksTask);
    }
    if (m_pCoverArtArchiveImageTask) {
        m_pCoverArtArchiveImageTask->invokeAbort();
        DEBUG_ASSERT(!m_pCoverArtArchiveImageTask);
    }
}

void TagFetcher::terminate() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    m_pTrack.reset();

    m_fingerprintWatcher.disconnect(this);
    m_fingerprintWatcher.cancel();

    if (m_pAcoustIdTask) {
        m_pAcoustIdTask->disconnect(this);
        m_pAcoustIdTask->deleteLater();
        m_pAcoustIdTask = nullptr;
    }

    if (m_pMusicBrainzTask) {
        m_pMusicBrainzTask->disconnect(this);
        m_pMusicBrainzTask->deleteLater();
        m_pMusicBrainzTask = nullptr;
    }

    if (m_pCoverArtArchiveImageTask) {
        m_pCoverArtArchiveImageTask->disconnect(this);
        m_pCoverArtArchiveImageTask->deleteLater();
        m_pCoverArtArchiveImageTask = nullptr;
    }

    if (m_pCoverArtArchiveLinksTask) {
        m_pCoverArtArchiveLinksTask->disconnect(this);
        m_pCoverArtArchiveLinksTask->deleteLater();
        m_pCoverArtArchiveLinksTask = nullptr;
    }
}

void TagFetcher::slotFingerprintReady() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!m_pTrack ||
            !m_fingerprintWatcher.isFinished()) {
        return;
    }

    DEBUG_ASSERT(m_fingerprintWatcher.isFinished());
    const QString fingerprint = m_fingerprintWatcher.result();
    if (fingerprint.isEmpty()) {
        emit resultAvailable(
                m_pTrack,
                {},
                tr("Reading track for fingerprinting failed."));
        return;
    }

    emit fetchProgress(tr("Identifying track through AcoustID"));
    DEBUG_ASSERT(!m_pAcoustIdTask);
    m_pAcoustIdTask = make_parented<mixxx::AcoustIdLookupTask>(
            &m_network,
            fingerprint,
            m_pTrack->getDurationSecondsInt(),
            this);
    connect(m_pAcoustIdTask,
            &mixxx::AcoustIdLookupTask::succeeded,
            this,
            &TagFetcher::slotAcoustIdTaskSucceeded);
    connect(m_pAcoustIdTask,
            &mixxx::AcoustIdLookupTask::failed,
            this,
            &TagFetcher::slotAcoustIdTaskFailed);
    connect(m_pAcoustIdTask,
            &mixxx::AcoustIdLookupTask::aborted,
            this,
            &TagFetcher::slotAcoustIdTaskAborted);
    connect(m_pAcoustIdTask,
            &mixxx::AcoustIdLookupTask::networkError,
            this,
            &TagFetcher::slotAcoustIdTaskNetworkError);
    m_pAcoustIdTask->invokeStart(
            kAcoustIdTimeoutMillis);
}

void TagFetcher::slotAcoustIdTaskSucceeded(
        QList<QUuid> recordingIds) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    auto* const pAcoustIdTask = m_pAcoustIdTask.get();
    VERIFY_OR_DEBUG_ASSERT(pAcoustIdTask ==
            qobject_cast<mixxx::AcoustIdLookupTask*>(sender())) {
        return;
    }
    m_pAcoustIdTask = nullptr;
    const auto taskDeleter = mixxx::ScopedDeleteLater(pAcoustIdTask);
    pAcoustIdTask->disconnect(this);

    if (recordingIds.isEmpty()) {
        auto pTrack = std::move(m_pTrack);
        terminate();

        emit resultAvailable(
                std::move(pTrack),
                {},
                tr("Could not identify track through AcoustID."));
        return;
    }

    emit fetchProgress(tr("Retrieving metadata from MusicBrainz"));
    emit numberOfRecordingsFoundFromAcoustId(recordingIds.size());

    DEBUG_ASSERT(!m_pMusicBrainzTask);
    m_pMusicBrainzTask = make_parented<mixxx::MusicBrainzRecordingsTask>(
            &m_network,
            std::move(recordingIds),
            this);
    connect(m_pMusicBrainzTask,
            &mixxx::MusicBrainzRecordingsTask::succeeded,
            this,
            &TagFetcher::slotMusicBrainzTaskSucceeded);
    connect(m_pMusicBrainzTask,
            &mixxx::MusicBrainzRecordingsTask::failed,
            this,
            &TagFetcher::slotMusicBrainzTaskFailed);
    connect(m_pMusicBrainzTask,
            &mixxx::MusicBrainzRecordingsTask::aborted,
            this,
            &TagFetcher::slotMusicBrainzTaskAborted);
    connect(m_pMusicBrainzTask,
            &mixxx::MusicBrainzRecordingsTask::networkError,
            this,
            &TagFetcher::slotMusicBrainzTaskNetworkError);
    connect(m_pMusicBrainzTask,
            &mixxx::MusicBrainzRecordingsTask::currentRecordingFetchedFromMusicBrainz,
            this,
            &TagFetcher::currentRecordingFetchedFromMusicBrainz);
    m_pMusicBrainzTask->invokeStart(
            kMusicBrainzTimeoutMillis);
}

void TagFetcher::slotAcoustIdTaskFailed(
        const mixxx::network::JsonWebResponse& response) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pAcoustIdTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }
    terminate();

    emit networkError(
            response.statusCode(),
            "AcoustID",
            response.content().toJson(),
            -1);
}

void TagFetcher::slotAcoustIdTaskAborted() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pAcoustIdTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }
    terminate();
}

void TagFetcher::slotAcoustIdTaskNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pAcoustIdTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }
    terminate();

    emit networkError(
            responseWithContent.statusCode(),
            QStringLiteral("AcoustID"),
            errorString,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskAborted() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pMusicBrainzTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }
    terminate();
}

void TagFetcher::slotMusicBrainzTaskNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pMusicBrainzTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }
    terminate();

    emit networkError(
            responseWithContent.statusCode(),
            QStringLiteral("MusicBrainz"),
            errorString,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskFailed(
        const mixxx::network::WebResponse& response,
        int errorCode,
        const QString& errorMessage) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pMusicBrainzTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }
    terminate();

    emit networkError(
            response.statusCode(),
            "MusicBrainz",
            errorMessage,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskSucceeded(
        const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pMusicBrainzTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }
    auto pTrack = m_pTrack;
    terminate();

    if (guessedTrackReleases.empty()) {
        emit resultAvailable(
                std::move(pTrack),
                {},
                tr("Could not find this track in the MusicBrainz database."));
    } else {
        emit resultAvailable(
                std::move(pTrack),
                std::move(guessedTrackReleases),
                {});
    }
}

void TagFetcher::startFetchCoverArtLinks(
        const QUuid& albumReleaseId) {
    // In here the progress message can be handled better.
    // emit fetchProgress(tr("Finding Possible Cover Arts on Cover Art Archive"));
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    terminate();

    m_pCoverArtArchiveLinksTask = make_parented<mixxx::CoverArtArchiveLinksTask>(
            &m_network,
            std::move(albumReleaseId),
            this);

    connect(m_pCoverArtArchiveLinksTask,
            &mixxx::CoverArtArchiveLinksTask::succeeded,
            this,
            &TagFetcher::slotCoverArtArchiveLinksTaskSucceeded);
    connect(m_pCoverArtArchiveLinksTask,
            &mixxx::CoverArtArchiveLinksTask::failed,
            this,
            &TagFetcher::slotCoverArtArchiveLinksTaskFailed);
    connect(m_pCoverArtArchiveLinksTask,
            &mixxx::CoverArtArchiveLinksTask::aborted,
            this,
            &TagFetcher::slotCoverArtArchiveLinksTaskAborted);
    connect(m_pCoverArtArchiveLinksTask,
            &mixxx::CoverArtArchiveLinksTask::networkError,
            this,
            &TagFetcher::slotCoverArtArchiveLinksTaskNetworkError);

    m_pCoverArtArchiveLinksTask->invokeStart(
            kCoverArtArchiveLinksTimeoutMilis);
}

void TagFetcher::slotCoverArtArchiveLinksTaskAborted() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pCoverArtArchiveLinksTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }

    terminate();
}

void TagFetcher::slotCoverArtArchiveLinksTaskNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    // TODO(XXX) Handle the error better for Cover Art Archive Links.
    Q_UNUSED(errorCode);
    Q_UNUSED(errorString);
    Q_UNUSED(responseWithContent);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pCoverArtArchiveLinksTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }

    emit coverArtLinkNotFound();
    terminate();
}

void TagFetcher::slotCoverArtArchiveLinksTaskFailed(
        const mixxx::network::JsonWebResponse& response) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pCoverArtArchiveLinksTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }

    terminate();

    emit networkError(
            response.statusCode(),
            "CoverArtArchive",
            response.content().toJson(),
            -1);
}

void TagFetcher::slotCoverArtArchiveLinksTaskSucceeded(const QUuid& albumReleaseId,
        const QList<QString>& allUrls) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pCoverArtArchiveLinksTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }

    auto pTrack = std::move(m_pTrack);
    terminate();

    emit coverArtArchiveLinksAvailable(std::move(albumReleaseId),
            std::move(allUrls));
}

void TagFetcher::startFetchCoverArtImage(const QUuid& albumReleaseId,
        const QString& coverArtUrl) {
    m_pCoverArtArchiveImageTask = make_parented<mixxx::CoverArtArchiveImageTask>(
            &m_network,
            coverArtUrl,
            albumReleaseId,
            this);

    connect(m_pCoverArtArchiveImageTask,
            &mixxx::CoverArtArchiveImageTask::succeeded,
            this,
            &TagFetcher::slotCoverArtArchiveImageTaskSucceeded);
    connect(m_pCoverArtArchiveImageTask,
            &mixxx::CoverArtArchiveImageTask::failed,
            this,
            &TagFetcher::slotCoverArtArchiveImageTaskFailed);
    connect(m_pCoverArtArchiveImageTask,
            &mixxx::CoverArtArchiveImageTask::aborted,
            this,
            &TagFetcher::slotCoverArtArchiveImageTaskAborted);
    connect(m_pCoverArtArchiveImageTask,
            &mixxx::CoverArtArchiveImageTask::networkError,
            this,
            &TagFetcher::slotCoverArtArchiveImageTaskNetworkError);

    m_pCoverArtArchiveImageTask->invokeStart(
            kCoverArtArchiveImageTimeoutMilis);
}

void TagFetcher::slotCoverArtArchiveImageTaskSucceeded(const QUuid& albumReleaseId,
        const QByteArray& coverArtBytes) {
    if (m_pCoverArtArchiveImageTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }

    auto pTrack = std::move(m_pTrack);
    terminate();

    emit coverArtImageFetchAvailable(std::move(albumReleaseId),
            coverArtBytes);
}

void TagFetcher::slotCoverArtArchiveImageTaskAborted() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pCoverArtArchiveImageTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }

    terminate();
}

void TagFetcher::slotCoverArtArchiveImageTaskFailed(
        const mixxx::network::WebResponse& response,
        int errorCode,
        const QString& errorMessage) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pCoverArtArchiveImageTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }

    terminate();

    emit networkError(
            response.statusCode(),
            "CoverArtImageTask",
            errorMessage,
            errorCode);
}

void TagFetcher::slotCoverArtArchiveImageTaskNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    Q_UNUSED(responseWithContent);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (m_pCoverArtArchiveImageTask.get() != sender()) {
        // stray call from an already aborted try
        return;
    }

    terminate();

    emit networkError(
            mixxx::network::kHttpStatusCodeInvalid,
            QStringLiteral("CoverArtArchive"),
            errorString,
            errorCode);
}
