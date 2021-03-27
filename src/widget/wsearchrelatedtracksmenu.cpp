#include "widget/wsearchrelatedtracksmenu.h"

#include <QScreen>

#include "moc_wsearchrelatedtracksmenu.cpp"
#include "track/track.h"
#include "util/math.h"
#include "util/qt.h"
#include "util/widgethelper.h"

namespace {

// Occupying up to 20% of the screen's width has been considered
// a viable upper bound for the context menu.
constexpr double kMaxMenuToAvailableScreenWidthRatio = 0.2;

constexpr double kRelativeBpmRange = 0.06; // +/-6 %

const QString kActionTextPrefixSuffixSeparator = QStringLiteral(" | ");

inline int bpmLowerBound(double bpm) {
    return static_cast<int>(std::floor((1 - kRelativeBpmRange) * bpm));
}

inline int bpmUpperBound(double bpm) {
    return static_cast<int>(std::ceil((1 + kRelativeBpmRange) * bpm));
}

inline QString quoteSearchQueryText(const QString& text) {
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
        /*!by-value-because-captured-by-lambda!*/
        QString searchQuery, // clazy:exclude=function-args-by-ref
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
            this,
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
    const auto* const pScreen =
            mixxx::widgethelper::getScreen(*this);
    VERIFY_OR_DEBUG_ASSERT(pScreen) {
        // This should never fail
        return actionTextPrefix;
    }
    const QString actionTextPrefixWithSeparator =
            actionTextPrefix + kActionTextPrefixSuffixSeparator;
    const auto prefixWidthInPixels =
            fontMetrics().boundingRect(actionTextPrefixWithSeparator).width();
    const auto maxWidthInPixels =
            math_max(
                    static_cast<int>(std::ceil(
                            pScreen->availableSize().width() *
                            kMaxMenuToAvailableScreenWidthRatio)),
                    prefixWidthInPixels);
    const auto elidedTextSuffix =
            fontMetrics().elidedText(
                    elidableTextSuffix,
                    // TODO: Customize the suffix elision?
                    Qt::ElideMiddle,
                    maxWidthInPixels - prefixWidthInPixels);
    return actionTextPrefixWithSeparator + elidedTextSuffix;
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
                    QStringLiteral("~key:") +
                    quoteSearchQueryText(keyText);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Key"),
                    tr("harmonic with %1").arg(keyText));
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
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("BPM"),
                    tr("between %1 and %2").arg(minBpmNumber, maxBpmNumber));
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
            {
                const auto actionTextPrefix = tr("Artist");
                const auto searchQueryPrefix = QStringLiteral("artist:");
                {
                    const QString searchQuery =
                            searchQueryPrefix +
                            quoteSearchQueryText(primaryArtist);
                    addTriggerSearchAction(
                            &addSeparatorBeforeNextAction,
                            searchQuery,
                            actionTextPrefix,
                            primaryArtist);
                }
                if (!secondaryArtist.isEmpty()) {
                    const QString searchQuery =
                            searchQueryPrefix +
                            quoteSearchQueryText(secondaryArtist);
                    addTriggerSearchAction(
                            &addSeparatorBeforeNextAction,
                            searchQuery,
                            actionTextPrefix,
                            secondaryArtist);
                }
            }
            {
                const auto actionTextPrefix = tr("Album Artist");
                const auto searchQueryPrefix = QStringLiteral("album_artist:");
                {
                    const QString searchQuery =
                            searchQueryPrefix +
                            quoteSearchQueryText(primaryArtist);
                    addTriggerSearchAction(
                            &addSeparatorBeforeNextAction,
                            searchQuery,
                            actionTextPrefix,
                            primaryArtist);
                }
                if (!secondaryArtist.isEmpty()) {
                    const QString searchQuery =
                            searchQueryPrefix +
                            quoteSearchQueryText(secondaryArtist);
                    addTriggerSearchAction(
                            &addSeparatorBeforeNextAction,
                            searchQuery,
                            actionTextPrefix,
                            secondaryArtist);
                }
            }
        }
    }
    {
        const auto composer = track.getComposer();
        if (!composer.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("composer:") +
                    quoteSearchQueryText(composer);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Composer"),
                    composer);
        }
    }

    // Title and grouping actions
    addSeparatorBeforeNextAction = !isEmpty();
    {
        const auto title = track.getTitle();
        if (!title.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("title:") +
                    quoteSearchQueryText(title);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Title"),
                    title);
        }
    }
    {
        const auto album = track.getAlbum();
        if (!album.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("album:") +
                    quoteSearchQueryText(album);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Album"),
                    album);
        }
    }
    {
        const auto grouping = track.getGrouping();
        if (!grouping.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("grouping:") +
                    quoteSearchQueryText(grouping);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Grouping"),
                    grouping);
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
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Year"),
                    releaseYearNumber);
        }
    }
    {
        const auto genre = track.getGenre();
        if (!genre.isEmpty()) {
            const QString searchQuery =
                    QStringLiteral("genre:") +
                    quoteSearchQueryText(genre);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Genre"),
                    genre);
        }
    }
    {
        const auto locationPath = track.getFileInfo().directory();
        if (!locationPath.isEmpty()) {
            // Search folder and all subfolders, i.e. for "path/to/folder"
            // also find files in "path/to/folder/subfolder" but not in
            // "path/to/folder copy".
            DEBUG_ASSERT(!locationPath.endsWith(QChar('/')));
            const QString locationPathWithTerminator =
                    locationPath + QChar('/');
            const QString searchQuery =
                    QStringLiteral("location:") +
                    quoteSearchQueryText(locationPathWithTerminator);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Folder"),
                    locationPathWithTerminator);
        }
    }
}
