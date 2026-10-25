#include "library/autodj/smart/genrescan.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <algorithm>

#include "library/autodj/smart/bridgefinder.h"

namespace genrescan {

QString searchTitle(const QString& title) {
    QString t = title;
    static const QRegularExpression kBrackets(QStringLiteral("[\\(\\[\\{][^\\)\\]\\}]*[\\)\\]\\}]"));
    t.remove(kBrackets);
    static const QRegularExpression kVersionEnding(QStringLiteral(
            "\\s[-\\x{2013}]\\s.*\\b(mix|remix|version|edit|extended|dub|instrumental|"
            "live|remaster|remastered|radio|club|single|album|original)\\b.*$"),
            QRegularExpression::CaseInsensitiveOption);
    t.remove(kVersionEnding);
    static const QRegularExpression kFeaturing(
            QStringLiteral("\\s(feat\\.?|ft\\.?|featuring)\\s.*$"),
            QRegularExpression::CaseInsensitiveOption);
    t.remove(kFeaturing);
    return t.simplified();
}

QString recordingQuery(const QString& artist, const QString& title) {
    const QString a = artist.trimmed();
    const QString t = searchTitle(title);
    if (a.isEmpty() || t.isEmpty()) {
        return QString();
    }
    const auto quoted = [](QString s) {
        s.replace(QChar('\\'), QStringLiteral("\\\\"));
        s.replace(QChar('"'), QStringLiteral("\\\""));
        return QChar('"') + s + QChar('"');
    };
    return QStringLiteral("recording:") + quoted(t) + QStringLiteral(" AND artist:") + quoted(a);
}

std::optional<Match> pickRecording(
        const QByteArray& searchJson, const QString& artist, const QString& title) {
    const QJsonArray recordings =
            QJsonDocument::fromJson(searchJson).object().value(QStringLiteral("recordings")).toArray();
    const QString ourTitle = BridgeFinder::normalizeTitle(title, BridgeFinder::normalizeArtist(artist));
    const QString ourArtist = BridgeFinder::normalizeArtist(artist);
    if (ourTitle.isEmpty() || ourArtist.isEmpty()) {
        return std::nullopt;
    }
    for (const QJsonValue& value : recordings) {
        const QJsonObject rec = value.toObject();
        const int score = rec.value(QStringLiteral("score")).toInt();
        if (score < kMinScore) {
            continue;
        }
        const QString theirTitle = rec.value(QStringLiteral("title")).toString();
        if (BridgeFinder::normalizeTitle(theirTitle, QString()) != ourTitle) {
            continue;
        }
        const QJsonArray credits = rec.value(QStringLiteral("artist-credit")).toArray();
        for (const QJsonValue& creditValue : credits) {
            const QJsonObject artistObj = creditValue.toObject().value(QStringLiteral("artist")).toObject();
            const QString theirArtist = BridgeFinder::normalizeArtist(
                    artistObj.value(QStringLiteral("name")).toString());
            if (theirArtist.isEmpty() ||
                    !(theirArtist.contains(ourArtist) || ourArtist.contains(theirArtist))) {
                continue;
            }
            Match m;
            m.recordingId = rec.value(QStringLiteral("id")).toString();
            m.artistId = artistObj.value(QStringLiteral("id")).toString();
            m.title = theirTitle;
            m.artist = artistObj.value(QStringLiteral("name")).toString();
            m.score = score;
            if (!m.recordingId.isEmpty()) {
                return m;
            }
        }
    }
    return std::nullopt;
}

QList<Genre> parseGenres(const QByteArray& lookupJson) {
    QList<Genre> genres;
    const QJsonArray array =
            QJsonDocument::fromJson(lookupJson).object().value(QStringLiteral("genres")).toArray();
    for (const QJsonValue& value : array) {
        const QJsonObject g = value.toObject();
        const QString name = g.value(QStringLiteral("name")).toString().trimmed();
        if (!name.isEmpty()) {
            genres.append(std::make_pair(name, g.value(QStringLiteral("count")).toInt()));
        }
    }
    return genres;
}

namespace {
std::optional<Genre> mostVoted(const QList<Genre>& genres) {
    if (genres.isEmpty()) {
        return std::nullopt;
    }
    // Highest count; the first one wins a tie (MusicBrainz order).
    return *std::max_element(genres.begin(), genres.end(), [](const Genre& a, const Genre& b) {
        return a.second < b.second;
    });
}
} // namespace

std::optional<Suggestion> pickGenre(
        const QList<Genre>& recordingGenres, const QList<Genre>& artistGenres) {
    if (const auto g = mostVoted(recordingGenres)) {
        return Suggestion{displayGenre(g->first), QStringLiteral("song"), g->second};
    }
    if (const auto g = mostVoted(artistGenres)) {
        return Suggestion{displayGenre(g->first), QStringLiteral("artist"), g->second};
    }
    return std::nullopt;
}

QString displayGenre(const QString& name) {
    static const QStringList kUpper = {QStringLiteral("ebm"),
            QStringLiteral("edm"),
            QStringLiteral("idm"),
            QStringLiteral("uk"),
            QStringLiteral("r&b")};
    QString out;
    QString word;
    const auto flush = [&]() {
        if (word.isEmpty()) {
            return;
        }
        if (kUpper.contains(word.toLower())) {
            out += word.toUpper();
        } else {
            out += word.left(1).toUpper() + word.mid(1).toLower();
        }
        word.clear();
    };
    for (const QChar c : name.trimmed()) {
        if (c == QChar(' ') || c == QChar('-') || c == QChar('/')) {
            flush();
            out += c;
        } else {
            word += c;
        }
    }
    flush();
    return out;
}

bool isEmptyGenre(const QString& genre) {
    const QString g = genre.trimmed().toLower();
    return g.isEmpty() || g == QStringLiteral("other") || g == QStringLiteral("unknown") ||
            g == QStringLiteral("misc") || g == QStringLiteral("none") ||
            g == QStringLiteral("genre");
}

} // namespace genrescan
