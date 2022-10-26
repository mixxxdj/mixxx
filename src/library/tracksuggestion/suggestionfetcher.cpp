#include "library/tracksuggestion/suggestionfetcher.h"

#include <QFuture>
#include <QString>

#include "track/track.h"
#include "util/thread_affinity.h"

namespace {

constexpr int kLastFmTimeoutMillis = 60000; // msec

QString composeButtonName(const QString& title, const QString& artist) {
    return artist + QStringLiteral(" | ") + title;
}

} // anonymous namespace

SuggestionFetcher::SuggestionFetcher(QObject* parent)
        : QObject(parent) {
}

void SuggestionFetcher::startFetch(
        TrackPointer pTrack) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    cancel();

    m_pTrack = pTrack;

    QString artist = pTrack->getArtist();
    QString title = pTrack->getTitle();
    emit fetchSuggestionProgress(
            tr("Fetching started for %1")
                    .arg(composeButtonName(title, artist)));

    m_pLastfmGetTrackSimilarTask = make_parented<mixxx::LastfmGetTrackSimilarTask>(
            &m_network,
            artist,
            title,
            this);

    connect(m_pLastfmGetTrackSimilarTask,
            &mixxx::LastfmGetTrackSimilarTask::succeeded,
            this,
            &SuggestionFetcher::slotLastfmGetTrackSimilarTaskSucceeded);
    connect(m_pLastfmGetTrackSimilarTask,
            &mixxx::LastfmGetTrackSimilarTask::failed,
            this,
            &SuggestionFetcher::slotLastfmGetTrackSimilarTaskFailed);
    connect(m_pLastfmGetTrackSimilarTask,
            &mixxx::LastfmGetTrackSimilarTask::aborted,
            this,
            &SuggestionFetcher::slotLastfmGetTrackSimilarTaskAborted);
    connect(m_pLastfmGetTrackSimilarTask,
            &mixxx::LastfmGetTrackSimilarTask::networkError,
            this,
            &SuggestionFetcher::slotLastfmGetTrackSimilarTaskNetworkError);
    m_pLastfmGetTrackSimilarTask->invokeStart(
            kLastFmTimeoutMillis);
}

void SuggestionFetcher::cancel() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    m_pTrack.reset();

    if (m_pLastfmGetTrackSimilarTask) {
        m_pLastfmGetTrackSimilarTask->invokeAbort();
        DEBUG_ASSERT(!m_pLastfmGetTrackSimilarTask);
    }
}

void SuggestionFetcher::slotLastfmGetTrackSimilarTaskSucceeded(
        const QList<QMap<QString, QString>>& suggestions) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onLastfmGetTrackSimilarTaskTerminated()) {
        return;
    }

    emit suggestionResults(suggestions);
    auto pTrack = std::move(m_pTrack);
    cancel();
}

bool SuggestionFetcher::onLastfmGetTrackSimilarTaskTerminated() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    auto* const pLastfmGetTrackSimilarTask = m_pLastfmGetTrackSimilarTask.get();
    DEBUG_ASSERT(sender());
    VERIFY_OR_DEBUG_ASSERT(pLastfmGetTrackSimilarTask ==
            qobject_cast<mixxx::LastfmGetTrackSimilarTask*>(sender())) {
        return false;
    }
    m_pLastfmGetTrackSimilarTask = nullptr;
    const auto taskDeleter = mixxx::ScopedDeleteLater(pLastfmGetTrackSimilarTask);
    pLastfmGetTrackSimilarTask->disconnect(this);
    return true;
}

void SuggestionFetcher::slotLastfmGetTrackSimilarTaskFailed(
        const mixxx::network::JsonWebResponse& response) {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onLastfmGetTrackSimilarTaskTerminated()) {
        return;
    }

    cancel();

    emit networkError(
            response.statusCode(),
            "Last Fm",
            response.content().toJson(),
            -1);
}

void SuggestionFetcher::slotLastfmGetTrackSimilarTaskAborted() {
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onLastfmGetTrackSimilarTaskTerminated()) {
        return;
    }

    cancel();
}

void SuggestionFetcher::slotLastfmGetTrackSimilarTaskNetworkError(
        QNetworkReply::NetworkError errorCode,
        const QString& errorString,
        const mixxx::network::WebResponseWithContent& responseWithContent) {
    Q_UNUSED(responseWithContent);
    DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
    if (!onLastfmGetTrackSimilarTaskTerminated()) {
        return;
    }

    cancel();

    emit networkError(
            mixxx::network::kHttpStatusCodeInvalid,
            QStringLiteral("Last.fm"),
            errorString,
            errorCode);
}

void SuggestionFetcher::transmitButtonLabel(TrackPointer pTrack) {
    QString artist = pTrack->getArtist();
    QString title = pTrack->getTitle();

    emit changeButtonLabel(composeButtonName(title, artist));
}
