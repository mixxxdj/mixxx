#include "musicmatcherclient.h"

#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

namespace mixxx {

static const Logger kLogger("MusicMatcherClient");

MusicMatcherClient::MusicMatcherClient(QObject* parent)
        : QObject(parent),
          m_pNam(new QNetworkAccessManager(this)),
          m_pTimer(new QTimer(this)) {
    m_pTimer->setSingleShot(true);
    m_pTimer->setInterval(15000);
    connect(m_pTimer, &QTimer::timeout, this, &MusicMatcherClient::slotTimeout);
}

MusicMatcherClient::~MusicMatcherClient() {
}

void MusicMatcherClient::findSimilar(const QString& artist, const QString& title, int limit) {
    m_suggestions.clear();
    m_originalArtist = artist;
    m_originalTitle = title;
    m_limit = limit;
    m_pendingArtistId.clear();

    if (artist.isEmpty() && title.isEmpty()) {
        emit searchFailed("Artist and title are both empty");
        return;
    }

    if (!title.isEmpty()) {
        searchTrack(artist, title);
    } else {
        searchArtist(artist);
    }
}

void MusicMatcherClient::searchTrack(const QString& artist, const QString& title) {
    m_pTimer->start();
    QUrl url(QStringLiteral("https://api.deezer.com/search"));
    QUrlQuery urlQuery;

    // Attempt strict search first
    QString queryStr;
    if (!artist.isEmpty()) {
        queryStr = QStringLiteral("track:\"%1\" artist:\"%2\"").arg(title, artist);
    } else {
        queryStr = title;
    }

    urlQuery.addQueryItem(QStringLiteral("q"), queryStr);
    urlQuery.addQueryItem(QStringLiteral("limit"), QStringLiteral("3"));
    url.setQuery(urlQuery);

    kLogger.info() << "MusicMatcher: searching track on Deezer:" << queryStr;

    QNetworkReply* pReply = m_pNam->get(QNetworkRequest(url));
    connect(pReply, &QNetworkReply::finished, this, [this, pReply, artist, title]() {
        pReply->deleteLater();
        m_pTimer->stop();

        if (pReply->error() != QNetworkReply::NoError) {
            kLogger.warning() << "MusicMatcher: track search failed, falling back to artist search:"
                              << pReply->errorString();
            searchArtist(artist);
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
        if (doc.isNull() || !doc.object().contains("data")) {
            searchArtist(artist);
            return;
        }

        const QJsonObject rootObj = doc.object();
        const QJsonArray data = rootObj["data"].toArray();
        if (data.isEmpty()) {
            kLogger.info() << "MusicMatcher: track not found on Deezer, "
                              "falling back to artist search";
            searchArtist(artist);
            return;
        }

        // Selected first matching track ID
        QString trackId = QString::number(data.first().toObject()["id"].toInteger());
        fetchTrackRadio(trackId);
    });
}

void MusicMatcherClient::fetchTrackRadio(const QString& trackId) {
    m_pTimer->start();
    QUrl url(QStringLiteral("https://api.deezer.com/track/%1/radio").arg(trackId));
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("limit"), QString::number(m_limit));
    url.setQuery(urlQuery);

    kLogger.info() << "MusicMatcher: fetching related tracks from Deezer track radio:" << trackId;

    QNetworkReply* pReply = m_pNam->get(QNetworkRequest(url));
    connect(pReply, &QNetworkReply::finished, this, [this, pReply]() {
        pReply->deleteLater();
        m_pTimer->stop();

        if (pReply->error() != QNetworkReply::NoError) {
            kLogger.warning() << "MusicMatcher: track radio search failed, "
                                 "falling back to artist search";
            searchArtist(m_originalArtist);
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
        if (doc.isNull() || !doc.object().contains("data")) {
            searchArtist(m_originalArtist);
            return;
        }

        const QJsonObject rootObj = doc.object();
        const QJsonArray data = rootObj["data"].toArray();
        if (data.isEmpty()) {
            searchArtist(m_originalArtist);
            return;
        }

        QList<MusicMatcherSuggestion> results;
        static const QRegularExpression kLiveRe(
                QStringLiteral("\\b(live|concert|performance|unplugged|acoustic)\\b"),
                QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression kInstrumentalRe(
                QStringLiteral("\\b(instrumental|karaoke|beat|type\\s*beat)\\b"),
                QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression kAwardRe(
                QStringLiteral("\\b(award|grammy|oscar|billboard|mtv|bet|ama)\\b"),
                QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression kFestivalRe(
                QStringLiteral("\\b(festival|coachella|lollapalooza|tomorrowland|ultra)\\b"),
                QRegularExpression::CaseInsensitiveOption);

        for (const QJsonValue& val : data) {
            QJsonObject obj = val.toObject();
            MusicMatcherSuggestion s;
            s.trackId = QString::number(obj["id"].toInteger());
            s.title = obj["title"].toString();

            QJsonObject artistObj = obj["artist"].toObject();
            s.artist = artistObj["name"].toString();

            QJsonObject albumObj = obj["album"].toObject();
            s.album = albumObj["title"].toString();

            s.duration = obj["duration"].toInt();
            s.sourceUrl = obj["link"].toString();

            QString titleLower = s.title.toLower();
            s.isLive = kLiveRe.match(titleLower).hasMatch();
            s.isInstrumental = kInstrumentalRe.match(titleLower).hasMatch();
            s.isAward = kAwardRe.match(titleLower).hasMatch();
            s.isFestival = kFestivalRe.match(titleLower).hasMatch();

            results.append(s);
        }

        m_suggestions = results;
        kLogger.info() << "MusicMatcher: found" << results.size() << "radio recommendations";
        emit suggestionsReady(results);
    });
}

void MusicMatcherClient::searchArtist(const QString& query) {
    m_pTimer->start();
    QUrl url(QStringLiteral("https://api.deezer.com/search/artist"));
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("q"), query);
    urlQuery.addQueryItem(QStringLiteral("limit"), QStringLiteral("5"));
    url.setQuery(urlQuery);

    kLogger.info() << "MusicMatcher: searching artist:" << query;

    QNetworkReply* pReply = m_pNam->get(QNetworkRequest(url));
    connect(pReply, &QNetworkReply::finished, this, [this, pReply]() {
        pReply->deleteLater();
        m_pTimer->stop();

        if (pReply->error() != QNetworkReply::NoError) {
            kLogger.warning() << "MusicMatcher: artist search failed:"
                              << pReply->errorString();
            emit searchFailed(pReply->errorString());
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
        if (doc.isNull() || !doc.object().contains("data")) {
            emit searchFailed("Invalid response from Deezer");
            return;
        }

        const QJsonObject rootObj = doc.object();
        const QJsonArray data = rootObj["data"].toArray();
        if (data.isEmpty()) {
            emit searchFailed("No artist found");
            return;
        }

        QString bestId;
        for (const QJsonValue& val : data) {
            QJsonObject obj = val.toObject();
            QString name = obj["name"].toString();
            if (name.compare(m_originalArtist, Qt::CaseInsensitive) == 0) {
                bestId = QString::number(obj["id"].toInteger());
                break;
            }
        }

        if (bestId.isEmpty()) {
            bestId = QString::number(data.first().toObject()["id"].toInteger());
        }

        m_pendingArtistId = bestId;
        fetchArtistTopTracks(bestId);
    });
}

void MusicMatcherClient::fetchArtistTopTracks(const QString& artistId) {
    m_pTimer->start();
    QUrl url(QStringLiteral("https://api.deezer.com/artist/%1/top").arg(artistId));
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("limit"), QString::number(m_limit));
    url.setQuery(urlQuery);

    kLogger.info() << "MusicMatcher: fetching top tracks for artist" << artistId;

    QNetworkReply* pReply = m_pNam->get(QNetworkRequest(url));
    connect(pReply, &QNetworkReply::finished, this, [this, pReply]() {
        pReply->deleteLater();
        m_pTimer->stop();

        if (pReply->error() != QNetworkReply::NoError) {
            kLogger.warning() << "MusicMatcher: top tracks failed:"
                              << pReply->errorString();
            emit searchFailed(pReply->errorString());
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(pReply->readAll());
        if (doc.isNull() || !doc.object().contains("data")) {
            emit searchFailed("Invalid response from Deezer");
            return;
        }

        const QJsonObject rootObj = doc.object();
        const QJsonArray data = rootObj["data"].toArray();
        QList<MusicMatcherSuggestion> results;

        static const QRegularExpression kLiveRe(
                QStringLiteral("\\b(live|concert|performance|unplugged|acoustic)\\b"),
                QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression kInstrumentalRe(
                QStringLiteral("\\b(instrumental|karaoke|beat|type\\s*beat)\\b"),
                QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression kAwardRe(
                QStringLiteral("\\b(award|grammy|oscar|billboard|mtv|bet|ama)\\b"),
                QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression kFestivalRe(
                QStringLiteral("\\b(festival|coachella|lollapalooza|tomorrowland|ultra)\\b"),
                QRegularExpression::CaseInsensitiveOption);

        for (const QJsonValue& val : data) {
            QJsonObject obj = val.toObject();
            MusicMatcherSuggestion s;
            s.trackId = QString::number(obj["id"].toInteger());
            s.title = obj["title"].toString();

            QJsonObject artistObj = obj["artist"].toObject();
            s.artist = artistObj["name"].toString();

            QJsonObject albumObj = obj["album"].toObject();
            s.album = albumObj["title"].toString();

            s.duration = obj["duration"].toInt();
            s.sourceUrl = obj["link"].toString();

            QString titleLower = s.title.toLower();
            s.isLive = kLiveRe.match(titleLower).hasMatch();
            s.isInstrumental = kInstrumentalRe.match(titleLower).hasMatch();
            s.isAward = kAwardRe.match(titleLower).hasMatch();
            s.isFestival = kFestivalRe.match(titleLower).hasMatch();

            results.append(s);
        }

        m_suggestions = results;
        kLogger.info() << "MusicMatcher: found" << results.size() << "suggestions";
        emit suggestionsReady(results);
    });
}

void MusicMatcherClient::slotTimeout() {
    kLogger.warning() << "MusicMatcher: request timed out";
    emit searchFailed("Request timed out");
}

void MusicMatcherClient::slotSearchFinished() {
    // Default handler for cases where the reply is finished but we haven't
    // processed it via the specialized slots. This should not normally
    // be reached since we use per-reply lambdas.
    kLogger.info() << "MusicMatcher: search finished (default handler)";
}

} // namespace mixxx

#include "moc_musicmatcherclient.cpp"
