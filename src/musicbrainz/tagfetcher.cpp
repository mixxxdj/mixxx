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

void TagFetcher::cancel() {
    m_pTrack.reset();
    m_fingerprintWatcher.cancel();
    if (m_pAcoustIdTask) {
        m_pAcoustIdTask->invokeAbort();
    }
    if (m_pMusicBrainzTask) {
        m_pMusicBrainzTask->invokeAbort();
    }
}

void TagFetcher::slotFingerprintReady() {
    if (!m_pTrack ||
            !m_fingerprintWatcher.isFinished()) {
        return;
    }

    const QString fingerprint = m_fingerprintWatcher.result();
    if (fingerprint.isEmpty()) {
        emit resultAvailable(
                m_pTrack,
                QList<mixxx::musicbrainz::TrackRelease>());
        return;
    }

    emit fetchProgress(tr("Identifying track through Acoustid"));
    DEBUG_ASSERT(!m_pAcoustIdTask);
    m_pAcoustIdTask = make_parented<mixxx::AcoustIdLookupTask>(
            &m_network,
            fingerprint,
            m_pTrack->getDurationInt(),
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
    VERIFY_OR_DEBUG_ASSERT(m_pAcoustIdTask) {
        return;
    }
    m_pAcoustIdTask->deleteAfterFinished();
    m_pAcoustIdTask = nullptr;

    if (!m_pTrack) {
        return;
    }

    if (recordingIds.isEmpty()) {
        emit resultAvailable(
                m_pTrack,
                QList<mixxx::musicbrainz::TrackRelease>());
        return;
    }

    emit fetchProgress(tr("Retrieving metadata from MusicBrainz"));
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
    m_pMusicBrainzTask->invokeStart(
            kMusicBrainzTimeoutMillis);
}

void TagFetcher::slotAcoustIdTaskFailed(
        mixxx::network::JsonWebResponse response) {
    VERIFY_OR_DEBUG_ASSERT(m_pAcoustIdTask) {
        return;
    }
    m_pAcoustIdTask->deleteAfterFinished();
    m_pAcoustIdTask = nullptr;
    emit networkError(
            response.statusCode,
            "AcoustID",
            response.content.toJson(),
            -1);
}

void TagFetcher::slotAcoustIdTaskAborted() {
    VERIFY_OR_DEBUG_ASSERT(m_pAcoustIdTask) {
        return;
    }
    m_pAcoustIdTask->deleteAfterFinished();
    m_pAcoustIdTask = nullptr;
}

void TagFetcher::slotAcoustIdTaskNetworkError(
        QUrl requestUrl,
        QNetworkReply::NetworkError errorCode,
        QString errorString,
        QByteArray errorContent) {
    Q_UNUSED(requestUrl);
    Q_UNUSED(errorContent);
    VERIFY_OR_DEBUG_ASSERT(m_pAcoustIdTask) {
        return;
    }
    m_pAcoustIdTask->deleteAfterFinished();
    m_pAcoustIdTask = nullptr;
    emit networkError(
            mixxx::network::kHttpStatusCodeInvalid,
            "AcoustID",
            errorString,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskAborted() {
    VERIFY_OR_DEBUG_ASSERT(m_pMusicBrainzTask) {
        return;
    }
    m_pMusicBrainzTask->deleteAfterFinished();
    m_pMusicBrainzTask = nullptr;
}

void TagFetcher::slotMusicBrainzTaskNetworkError(
        QUrl requestUrl,
        QNetworkReply::NetworkError errorCode,
        QString errorString,
        QByteArray errorContent) {
    Q_UNUSED(requestUrl);
    Q_UNUSED(errorContent);
    VERIFY_OR_DEBUG_ASSERT(m_pMusicBrainzTask) {
        return;
    }
    m_pMusicBrainzTask->deleteAfterFinished();
    m_pMusicBrainzTask = nullptr;
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
    VERIFY_OR_DEBUG_ASSERT(m_pMusicBrainzTask) {
        return;
    }
    m_pMusicBrainzTask->deleteAfterFinished();
    m_pMusicBrainzTask = nullptr;
    emit networkError(
            response.statusCode,
            "MusicBrainz",
            errorMessage,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskSucceeded(
        QList<mixxx::musicbrainz::TrackRelease> guessedTrackReleases) {
    auto pOriginalTrack = std::move(m_pTrack);
    DEBUG_ASSERT(!m_pTrack);
    VERIFY_OR_DEBUG_ASSERT(m_pMusicBrainzTask) {
        return;
    }
    m_pMusicBrainzTask->deleteAfterFinished();
    m_pMusicBrainzTask = nullptr;
    if (!pOriginalTrack) {
        // aborted
        return;
    }
    emit resultAvailable(
            pOriginalTrack,
            std::move(guessedTrackReleases));
}
