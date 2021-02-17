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

} // anonymous namespace

TagFetcher::TagFetcher(QObject* parent)
        : QObject(parent),
          m_fingerprintWatcher(this) {
}

void TagFetcher::startFetch(
        TrackPointer pTrack) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    cancel();

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
        cancel();

        emit resultAvailable(
                std::move(pTrack),
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

bool TagFetcher::onAcoustIdTaskTerminated() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    auto* const pAcoustIdTask = m_pAcoustIdTask.get();
    DEBUG_ASSERT(sender());
    VERIFY_OR_DEBUG_ASSERT(pAcoustIdTask ==
            qobject_cast<mixxx::AcoustIdLookupTask*>(sender())) {
        return false;
    }
    m_pAcoustIdTask = nullptr;
    const auto taskDeleter = mixxx::ScopedDeleteLater(pAcoustIdTask);
    pAcoustIdTask->disconnect(this);
    return true;
}

void TagFetcher::slotAcoustIdTaskFailed(
        const mixxx::network::JsonWebResponse& response) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onAcoustIdTaskTerminated()) {
        return;
    }

    cancel();

    emit networkError(
            response.statusCode(),
            "AcoustID",
            response.content().toJson(),
            -1);
}

void TagFetcher::slotAcoustIdTaskAborted() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onAcoustIdTaskTerminated()) {
        return;
    }

    cancel();
}

void TagFetcher::slotAcoustIdTaskNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    Q_UNUSED(responseWithContent);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onAcoustIdTaskTerminated()) {
        return;
    }

    cancel();

    emit networkError(
            mixxx::network::kHttpStatusCodeInvalid,
            QStringLiteral("AcoustID"),
            errorString,
            errorCode);
}

bool TagFetcher::onMusicBrainzTaskTerminated() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    auto* const pMusicBrainzTask = m_pMusicBrainzTask.get();
    DEBUG_ASSERT(sender());
    VERIFY_OR_DEBUG_ASSERT(pMusicBrainzTask ==
            qobject_cast<mixxx::MusicBrainzRecordingsTask*>(sender())) {
        return false;
    }
    m_pMusicBrainzTask = nullptr;
    const auto taskDeleter = mixxx::ScopedDeleteLater(pMusicBrainzTask);
    pMusicBrainzTask->disconnect(this);
    return true;
}

void TagFetcher::slotMusicBrainzTaskAborted() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onMusicBrainzTaskTerminated()) {
        return;
    }

    cancel();
}

void TagFetcher::slotMusicBrainzTaskNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    Q_UNUSED(responseWithContent);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onMusicBrainzTaskTerminated()) {
        return;
    }

    cancel();

    emit networkError(
            mixxx::network::kHttpStatusCodeInvalid,
            QStringLiteral("MusicBrainz"),
            errorString,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskFailed(
        const mixxx::network::WebResponse& response,
        int errorCode,
        const QString& errorMessage) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onMusicBrainzTaskTerminated()) {
        return;
    }

    cancel();

    emit networkError(
            response.statusCode(),
            "MusicBrainz",
            errorMessage,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskSucceeded(
        const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onMusicBrainzTaskTerminated()) {
        return;
    }

    auto pTrack = std::move(m_pTrack);
    cancel();

    emit resultAvailable(
            std::move(pTrack),
            std::move(guessedTrackReleases));
}
