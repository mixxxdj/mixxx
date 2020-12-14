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
        m_pAcoustIdTask->disconnect(this);
        m_pAcoustIdTask->deleteLater();
        m_pAcoustIdTask = nullptr;
    }
    if (m_pMusicBrainzTask) {
        m_pMusicBrainzTask->disconnect(this);
        m_pMusicBrainzTask->deleteLater();
        m_pMusicBrainzTask = nullptr;
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
    DEBUG_ASSERT(m_pAcoustIdTask.get() ==
            qobject_cast<mixxx::AcoustIdLookupTask*>(sender()));

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

void TagFetcher::slotAcoustIdTaskFailed(
        const mixxx::network::JsonWebResponse& response) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(m_pAcoustIdTask.get() ==
            qobject_cast<mixxx::AcoustIdLookupTask*>(sender()));

    cancel();

    emit networkError(
            response.statusCode,
            "AcoustID",
            response.content.toJson(),
            -1);
}

void TagFetcher::slotAcoustIdTaskAborted() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(m_pAcoustIdTask.get() ==
            qobject_cast<mixxx::AcoustIdLookupTask*>(sender()));

    auto pTrack = std::move(m_pTrack);
    cancel();

    emit resultAvailable(
            std::move(pTrack),
            QList<mixxx::musicbrainz::TrackRelease>{});
}

void TagFetcher::slotAcoustIdTaskNetworkError(
        const QUrl& requestUrl,
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const QByteArray& errorContent) {
    Q_UNUSED(requestUrl);
    Q_UNUSED(errorContent);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(m_pAcoustIdTask.get() ==
            qobject_cast<mixxx::AcoustIdLookupTask*>(sender()));

    cancel();

    emit networkError(
            mixxx::network::kHttpStatusCodeInvalid,
            "AcoustID",
            errorString,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskAborted() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(m_pMusicBrainzTask.get() ==
            qobject_cast<mixxx::MusicBrainzRecordingsTask*>(sender()));

    auto pTrack = std::move(m_pTrack);
    cancel();

    emit resultAvailable(
            std::move(pTrack),
            QList<mixxx::musicbrainz::TrackRelease>{});
}

void TagFetcher::slotMusicBrainzTaskNetworkError(
        const QUrl& requestUrl,
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const QByteArray& errorContent) {
    Q_UNUSED(requestUrl);
    Q_UNUSED(errorContent);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(m_pMusicBrainzTask.get() ==
            qobject_cast<mixxx::MusicBrainzRecordingsTask*>(sender()));

    cancel();

    emit networkError(
            mixxx::network::kHttpStatusCodeInvalid,
            "MusicBrainz",
            errorString,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskFailed(
        const mixxx::network::WebResponse& response,
        int errorCode,
        const QString& errorMessage) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(m_pMusicBrainzTask.get() ==
            qobject_cast<mixxx::MusicBrainzRecordingsTask*>(sender()));

    cancel();

    emit networkError(
            response.statusCode,
            "MusicBrainz",
            errorMessage,
            errorCode);
}

void TagFetcher::slotMusicBrainzTaskSucceeded(
        const QList<mixxx::musicbrainz::TrackRelease>& guessedTrackReleases) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    DEBUG_ASSERT(m_pMusicBrainzTask.get() ==
            qobject_cast<mixxx::MusicBrainzRecordingsTask*>(sender()));

    auto pTrack = std::move(m_pTrack);
    cancel();

    emit resultAvailable(
            std::move(pTrack),
            std::move(guessedTrackReleases));
}
