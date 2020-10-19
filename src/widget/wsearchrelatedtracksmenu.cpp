#include "widget/wsearchrelatedtracksmenu.h"

#include "track/track.h"
#include "util/qt.h"

namespace {

constexpr double kRelativeBpmRange = 0.06; // +/-6 %

inline int bpmLowerBound(double bpm) {
    return static_cast<int>(std::floor((1 - kRelativeBpmRange) * bpm));
}

inline int bpmUpperBound(double bpm) {
    return static_cast<int>(std::ceil((1 + kRelativeBpmRange) * bpm));
}

QString extractCalendarYearNumberFromReleaseDate(
        const QString& releaseDate) {
    // TODO: Improve this poor calendar year number parser
    // and extract the code
    int skippedLeadingZeros = 0;
    int countedLeadingDigits = 0;
    const auto trimmed = releaseDate.trimmed();
    // Count leading digits
    for (int i = 0; i < trimmed.size(); ++i) {
        if (!trimmed[i].isDigit()) {
            break;
        }
        // Skip leading zeros
        if (countedLeadingDigits == 0 && trimmed[i] == QChar('0')) {
            ++skippedLeadingZeros;
            continue;
        }
        // Count leading digits
        ++countedLeadingDigits;
    }
    // Interpret the leading digits as the calendar year
    // without any further validation
    return trimmed.mid(skippedLeadingZeros, countedLeadingDigits);
}

} // namespace

WSearchRelatedTracksMenu::WSearchRelatedTracksMenu(
        QWidget* parent)
        : QMenu(tr("Search related Tracks"), parent) {
}

bool WSearchRelatedTracksMenu::addTriggerSearchAction(
        bool addSeparatorBeforeAction,
        const QString& actionText,
        QString /*!by-value-because-captured-by-lambda!*/ searchQuery) {
    if (addSeparatorBeforeAction) {
        addSeparator();
    }
    addAction(
            mixxx::escapeTextPropertyWithoutShortcuts(actionText),
            [this, searchQuery]() {
                emit triggerSearch(searchQuery);
            });
    return false;
}

void WSearchRelatedTracksMenu::addActionsForTrack(
        const Track& track) {
    bool addSeparatorBeforeAction = false;

    // NOTE: We have to explicitly use `QString` instead of `auto`
    // when composing search queries using `QStringBuilder`. Otherwise
    // string concatenation will fail at runtime!

    // Musical property actions
    {
        const auto bpm = track.getBpm();
        if (bpm > 0) {
            const auto minBpmNumber = QString::number(bpmLowerBound(bpm));
            const auto maxBpmNumber = QString::number(bpmUpperBound(bpm));
            const auto actionText =
                    tr("Tempo between %1 and %2 bpm").arg(minBpmNumber, maxBpmNumber);
            const QString searchQuery =
                    QStringLiteral("bpm:>=") +
                    minBpmNumber +
                    QStringLiteral(" bpm:<=") +
                    maxBpmNumber;
            addSeparatorBeforeAction = addTriggerSearchAction(
                    addSeparatorBeforeAction,
                    actionText,
                    searchQuery);
        }
    }
    {
        const auto keyText = track.getKeyText();
        if (!keyText.isEmpty()) {
            const auto actionText =
                    tr("Harmonic keys for \"%1\"").arg(keyText);
            const QString searchQuery =
                    QStringLiteral("~key:\"") +
                    keyText +
                    QChar('"');
            addSeparatorBeforeAction = addTriggerSearchAction(
                    addSeparatorBeforeAction,
                    actionText,
                    searchQuery);
        }
    }
    {
        const auto genre = track.getGenre();
        if (!genre.isEmpty()) {
            const auto actionText =
                    tr("Genre \"%1\"").arg(genre);
            const QString searchQuery =
                    QStringLiteral("genre:\"") +
                    genre +
                    QChar('"');
            addSeparatorBeforeAction = addTriggerSearchAction(
                    addSeparatorBeforeAction,
                    actionText,
                    searchQuery);
        }
    }

    // Artist actions
    addSeparatorBeforeAction = true;
    {
        auto primaryArtist = track.getArtist();
        auto secondaryArtist = track.getAlbumArtist();
        if (primaryArtist.isEmpty()) {
            primaryArtist = secondaryArtist;
            secondaryArtist = QString();
        } else {
            if (!secondaryArtist.isEmpty() &&
                    primaryArtist.contains(secondaryArtist)) {
                // Use the shorter string as primary artist and the
                // longer string as secondary artist
                if (primaryArtist == secondaryArtist) {
                    secondaryArtist = QString();
                } else {
                    std::swap(primaryArtist, secondaryArtist);
                }
            }
        }
        DEBUG_ASSERT(!primaryArtist.isEmpty() || secondaryArtist.isEmpty());
        if (!primaryArtist.isEmpty()) {
            // Search tracks with similar artist(s)
            {
                const auto actionText =
                        tr("Artist \"%1\"").arg(primaryArtist);
                const QString searchQuery =
                        QStringLiteral("artist:\"") +
                        primaryArtist +
                        QChar('"');
                addSeparatorBeforeAction = addTriggerSearchAction(
                        addSeparatorBeforeAction,
                        actionText,
                        searchQuery);
            }
            if (!secondaryArtist.isEmpty()) {
                const auto actionText =
                        tr("Artist \"%1\"").arg(secondaryArtist);
                const QString searchQuery =
                        QStringLiteral("artist:\"") +
                        secondaryArtist +
                        QChar('"');
                addSeparatorBeforeAction = addTriggerSearchAction(
                        addSeparatorBeforeAction,
                        actionText,
                        searchQuery);
            }
            {
                const auto actionText =
                        tr("Album artist \"%1\"").arg(primaryArtist);
                const QString searchQuery =
                        QStringLiteral("album_artist:\"") +
                        primaryArtist +
                        QChar('"');
                addSeparatorBeforeAction = addTriggerSearchAction(
                        addSeparatorBeforeAction,
                        actionText,
                        searchQuery);
            }
            if (!secondaryArtist.isEmpty()) {
                const auto actionText =
                        tr("Album artist \"%1\"").arg(secondaryArtist);
                const QString searchQuery =
                        QStringLiteral("album_artist:\"") +
                        secondaryArtist +
                        QChar('"');
                addSeparatorBeforeAction = addTriggerSearchAction(
                        addSeparatorBeforeAction,
                        actionText,
                        searchQuery);
            }
        }
    }
    {
        const auto composer = track.getComposer();
        if (!composer.isEmpty()) {
            const auto actionText =
                    tr("Composer \"%1\"").arg(composer);
            const QString searchQuery =
                    QStringLiteral("composer:\"") +
                    composer +
                    QChar('"');
            addSeparatorBeforeAction = addTriggerSearchAction(
                    addSeparatorBeforeAction,
                    actionText,
                    searchQuery);
        }
    }

    // Release actions
    addSeparatorBeforeAction = true;
    {
        const auto title = track.getTitle();
        if (!title.isEmpty()) {
            const auto actionText =
                    tr("Title \"%1\"").arg(title);
            const QString searchQuery =
                    QStringLiteral("title:\"") +
                    title +
                    QChar('"');
            addSeparatorBeforeAction = addTriggerSearchAction(
                    addSeparatorBeforeAction,
                    actionText,
                    searchQuery);
        }
    }
    {
        const auto album = track.getAlbum();
        if (!album.isEmpty()) {
            const auto actionText =
                    tr("Album \"%1\"").arg(album);
            const QString searchQuery =
                    QStringLiteral("album:\"") +
                    album +
                    QChar('"');
            addSeparatorBeforeAction = addTriggerSearchAction(
                    addSeparatorBeforeAction,
                    actionText,
                    searchQuery);
        }
    }
    {
        const auto grouping = track.getGrouping();
        if (!grouping.isEmpty()) {
            const auto actionText =
                    tr("Grouping \"%1\"").arg(grouping);
            const QString searchQuery =
                    QStringLiteral("grouping:\"") +
                    grouping +
                    QChar('"');
            addSeparatorBeforeAction = addTriggerSearchAction(
                    addSeparatorBeforeAction,
                    actionText,
                    searchQuery);
        }
    }
    {
        const auto releaseYearNumber =
                extractCalendarYearNumberFromReleaseDate(track.getYear());
        if (!releaseYearNumber.isEmpty()) {
            const auto actionText =
                    tr("Release year %1").arg(releaseYearNumber);
            const QString searchQuery =
                    QStringLiteral("year:") +
                    releaseYearNumber;
            addSeparatorBeforeAction = addTriggerSearchAction(
                    addSeparatorBeforeAction,
                    actionText,
                    searchQuery);
        }
    }

    // File system actions
    addSeparatorBeforeAction = true;
    {
        const auto locationPath = track.getFileInfo().directory();
        if (!locationPath.isEmpty()) {
            const auto actionText =
                    tr("Folder \"%1\"").arg(locationPath);
            const QString searchQuery =
                    QStringLiteral("location:\"") +
                    locationPath +
                    QChar('"');
            addSeparatorBeforeAction = addTriggerSearchAction(
                    addSeparatorBeforeAction,
                    actionText,
                    searchQuery);
        }
    }
}
