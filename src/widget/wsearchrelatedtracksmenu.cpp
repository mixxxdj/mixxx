#include "widget/wsearchrelatedtracksmenu.h"

#include <QScreen>

#include "track/track.h"
#include "util/math.h"
#include "util/qt.h"
#include "util/widgethelper.h"

namespace {

constexpr double kMaxMenuToAvailableScreenWidthRatio = 0.2; // 20%

constexpr int kMinMenuWidthInPixels = 100;

constexpr double kRelativeBpmRange = 0.06; // +/-6 %

inline int bpmLowerBound(double bpm) {
    return static_cast<int>(std::floor((1 - kRelativeBpmRange) * bpm));
}

inline int bpmUpperBound(double bpm) {
    return static_cast<int>(std::ceil((1 + kRelativeBpmRange) * bpm));
}

inline QString quoteText(const QString& text) {
    return QChar('"') + text + QChar('"');
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

void WSearchRelatedTracksMenu::addTriggerSearchAction(
        bool* /*in/out*/ pAddSeparatorBeforeNextAction,
        QString /*!by-value-because-captured-by-lambda!*/ searchQuery,
        const QString& actionTextPrefix,
        const QString& elidableTextSuffix) {
    DEBUG_ASSERT(pAddSeparatorBeforeNextAction);
    if (*pAddSeparatorBeforeNextAction) {
        addSeparator();
    }
    // Reset the flag before adding the next action
    *pAddSeparatorBeforeNextAction = false;
    const auto elidedActionText =
            elideActionText(
                    actionTextPrefix,
                    elidableTextSuffix);
    addAction(
            mixxx::escapeTextPropertyWithoutShortcuts(elidedActionText),
            [this, searchQuery]() {
                emit triggerSearch(searchQuery);
            });
}

QString WSearchRelatedTracksMenu::elideActionText(
        const QString& actionTextPrefix,
        const QString& elidableTextSuffix) const {
    if (elidableTextSuffix.isEmpty()) {
        return actionTextPrefix;
    }
    const auto prefixWidthInPixels =
            fontMetrics().boundingRect(actionTextPrefix).width();
    const auto minWidthInPixels =
            math_max(
                    prefixWidthInPixels,
                    kMinMenuWidthInPixels);
    const auto* const pScreen =
            mixxx::widgethelper::getScreen(*this);
    VERIFY_OR_DEBUG_ASSERT(pScreen) {
        // This should never fail
        return actionTextPrefix;
    }
    const auto maxWidthInPixels =
            math_max(
                    static_cast<int>(
                            pScreen->availableSize().width() *
                            kMaxMenuToAvailableScreenWidthRatio),
                    minWidthInPixels);
    const auto elidedTextSuffix =
            fontMetrics().elidedText(
                    elidableTextSuffix,
                    // Most suffix text is quoted and should always
                    // be elided in the middle
                    Qt::ElideMiddle,
                    maxWidthInPixels - prefixWidthInPixels);
    return actionTextPrefix + elidedTextSuffix;
}

void WSearchRelatedTracksMenu::addActionsForTrack(
        const Track& track) {
    // NOTE: We have to explicitly use `QString` instead of `auto`
    // when composing search queries using `QStringBuilder`. Otherwise
    // string concatenation will fail at runtime!

    // Mixing actions
    bool addSeparatorBeforeNextAction = !isEmpty();
    {
        const auto keyText = track.getKeyText();
        if (!keyText.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("~key:\"") +
                    keyText +
                    QChar('"');
            const auto actionText =
                    tr("Key: Harmonic with \"%1\"").arg(keyText);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    actionText);
        }
    }
    {
        const auto bpm = track.getBpm();
        if (bpm > 0) {
            const auto minBpmNumber = QString::number(bpmLowerBound(bpm));
            const auto maxBpmNumber = QString::number(bpmUpperBound(bpm));
            const QString searchQuery =
                    QStringLiteral("bpm:>=") +
                    minBpmNumber +
                    QStringLiteral(" bpm:<=") +
                    maxBpmNumber;
            const auto actionText =
                    tr("BPM: Between %1 and %2").arg(minBpmNumber, maxBpmNumber);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    actionText);
        }
    }

    // Artist actions
    addSeparatorBeforeNextAction = !isEmpty();
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
            const auto artistTextPrefix = tr("Artist: ");
            const auto artistQueryPrefix = QStringLiteral("artist:\"");
            const auto querySuffix = QChar('"');
            {
                const QString searchQuery =
                        artistQueryPrefix +
                        primaryArtist +
                        querySuffix;
                addTriggerSearchAction(
                        &addSeparatorBeforeNextAction,
                        searchQuery,
                        artistTextPrefix,
                        quoteText(primaryArtist));
            }
            if (!secondaryArtist.isEmpty()) {
                const QString searchQuery =
                        artistQueryPrefix +
                        secondaryArtist +
                        querySuffix;
                addTriggerSearchAction(
                        &addSeparatorBeforeNextAction,
                        searchQuery,
                        artistTextPrefix,
                        quoteText(secondaryArtist));
            }
            const auto albumArtistTextPrefix = tr("Album Artist: ");
            const auto albumArtistQueryPrefix = QStringLiteral("album_artist:\"");
            {
                const QString searchQuery =
                        albumArtistQueryPrefix +
                        primaryArtist +
                        querySuffix;
                addTriggerSearchAction(
                        &addSeparatorBeforeNextAction,
                        searchQuery,
                        albumArtistTextPrefix,
                        quoteText(primaryArtist));
            }
            if (!secondaryArtist.isEmpty()) {
                const QString searchQuery =
                        albumArtistQueryPrefix +
                        secondaryArtist +
                        querySuffix;
                addTriggerSearchAction(
                        &addSeparatorBeforeNextAction,
                        searchQuery,
                        albumArtistTextPrefix,
                        quoteText(secondaryArtist));
            }
        }
    }
    {
        const auto composer = track.getComposer();
        if (!composer.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("composer:\"") +
                    composer +
                    QChar('"');
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Composer: "),
                    quoteText(composer));
        }
    }

    // Title and grouping actions
    addSeparatorBeforeNextAction = !isEmpty();
    {
        const auto title = track.getTitle();
        if (!title.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("title:\"") +
                    title +
                    QChar('"');
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Title: "),
                    quoteText(title));
        }
    }
    {
        const auto album = track.getAlbum();
        if (!album.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("album:\"") +
                    album +
                    QChar('"');
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Album: "),
                    quoteText(album));
        }
    }
    {
        const auto grouping = track.getGrouping();
        if (!grouping.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("grouping:\"") +
                    grouping +
                    QChar('"');
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Grouping: "),
                    quoteText(grouping));
        }
    }

    // Other actions
    addSeparatorBeforeNextAction = !isEmpty();
    {
        const auto releaseYearNumber =
                extractCalendarYearNumberFromReleaseDate(track.getYear());
        if (!releaseYearNumber.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("year:") +
                    releaseYearNumber;
            const auto actionText =
                    tr("Year: %1").arg(releaseYearNumber);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    actionText);
        }
    }
    {
        const auto genre = track.getGenre();
        if (!genre.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("genre:\"") +
                    genre +
                    QChar('"');
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Genre: "),
                    quoteText(genre));
        }
    }
    {
        const auto locationPath = track.getFileInfo().directory();
        if (!locationPath.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("location:\"") +
                    locationPath +
                    QChar('"');
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Folder: "),
                    quoteText(locationPath));
        }
    }
}
