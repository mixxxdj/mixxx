#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <optional>
#include <utility>

/// Auto DJ 2.0 plus Video Mixing Genre Scan: the pure part (no network, no Mixxx types), so
/// it can be unit-tested. GenreScanner does the web requests.
///
/// For each song: search MusicBrainz for the recording by artist + title,
/// accept only a close match, then take the recording's most-voted genre,
/// or the artist's most-voted genre if the recording has none.
/// MusicBrainz genres are community votes: a suggestion, not a fact, so the
/// DJ reviews every one before it is applied.
namespace genrescan {

/// A genre with its number of votes.
using Genre = std::pair<QString, int>;

/// The recording found for a song.
struct Match {
    QString recordingId;
    QString artistId;
    QString title;  ///< as MusicBrainz has it
    QString artist; ///< as MusicBrainz has it
    int score = 0;  ///< MusicBrainz search score 0..100
};

struct Suggestion {
    QString genre;  ///< display form, e.g. "Gothic Rock", "EBM"
    QString source; ///< "song" or "artist"
    int votes = 0;
};

/// The title without version tags, for the search ("Fade To Grey (12\"
/// Version)" -> "Fade To Grey").
QString searchTitle(const QString& title);

/// Lucene query for /ws/2/recording, or empty if artist or title is missing.
QString recordingQuery(const QString& artist, const QString& title);

/// Minimum MusicBrainz search score to accept a result.
constexpr int kMinScore = 85;

/// The best result of a /ws/2/recording search that really is this song:
/// score >= kMinScore, same title (after tidying) and same artist (one name
/// contains the other, after tidying). nullopt if none.
std::optional<Match> pickRecording(
        const QByteArray& searchJson, const QString& artist, const QString& title);

/// The "genres" array of a recording or artist lookup (inc=genres).
QList<Genre> parseGenres(const QByteArray& lookupJson);

/// The genre to suggest: the recording's most-voted, else the artist's.
std::optional<Suggestion> pickGenre(
        const QList<Genre>& recordingGenres, const QList<Genre>& artistGenres);

/// "gothic rock" -> "Gothic Rock", "synth-pop" -> "Synth-Pop", "ebm" -> "EBM".
QString displayGenre(const QString& name);

/// True for "", "Other", "Unknown" and similar: nothing useful set yet.
bool isEmptyGenre(const QString& genre);

} // namespace genrescan
