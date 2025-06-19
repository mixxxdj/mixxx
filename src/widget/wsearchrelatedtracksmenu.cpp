#include "widget/wsearchrelatedtracksmenu.h"

#include <QCheckBox>
#include <QMouseEvent>
#include <QPalette>
#include <QScreen>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QWidgetAction>

#include "library/searchquery.h"
#include "moc_wsearchrelatedtracksmenu.cpp"
#include "track/track.h"
#include "util/math.h"
#include "util/parented_ptr.h"
#include "util/qt.h"
#include "util/widgethelper.h"

namespace {

// Occupying up to 20% of the screen's width has been considered
// a viable upper bound for the context menu.
constexpr double kMaxMenuToAvailableScreenWidthRatio = 0.2;

const QString kActionTextPrefixSuffixSeparator = QStringLiteral(": ");

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
        QWidget* pParent)
        : QMenu(tr("Search related Tracks"), pParent) {
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

    auto pCheckBox = make_parented<WSearchRelatedCheckBox>(
            mixxx::escapeTextPropertyWithoutShortcuts(elidedActionText),
            this);
    pCheckBox->setProperty("query", searchQuery);
    connect(pCheckBox.get(),
            &QCheckBox::toggled,
            this,
            &WSearchRelatedTracksMenu::updateSearchButton);
    // Use the event filter to capture clicks on the checkbox label
    pCheckBox.get()->installEventFilter(this);

    auto pAction = make_parented<QWidgetAction>(this);
    pAction->setDefaultWidget(pCheckBox.get());
    // While the checkbox is selected (via keyboard, not hovered by pointer)
    // pressing Space will toggle it whereas pressing Return triggers the action.
    connect(pAction.get(),
            &QAction::triggered,
            this,
            [this, searchQuery]() {
                // TODO Call combineQueriesTriggerSearch() so we can check multiple
                // actions and press return on the last selected action without
                // having to go to the Search button?
                emit triggerSearch(searchQuery);
            });
    addAction(pAction.get());
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
    // Add some margin between the label and the separator bar (see paintEvent())
    return QStringLiteral("  ") + actionTextPrefixWithSeparator + elidedTextSuffix;
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
            QString bpmStr = QString::number(bpm);
            // BpmFilterNode has the user value for the fuzzy range, set in DlgPrefLibrary
            BpmFilterNode* pBpmNode = new BpmFilterNode(bpmStr, true /* fuzzy search */);
            double bpmLowerBound = 0.0;
            double bpmUpperBound = 0.0;
            std::tie(bpmLowerBound, bpmUpperBound) = pBpmNode->getBpmRange();
            const QString searchQuery =
                    QStringLiteral("bpm:>=") +
                    QString::number(bpmLowerBound) +
                    QStringLiteral(" bpm:<=") +
                    QString::number(bpmUpperBound);
            addTriggerSearchAction(&addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("BPM"),
                    tr("between %1 and %2")
                            .arg(QString::number(bpmLowerBound),
                                    QString::number(bpmUpperBound)));
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
        const auto locationPath = track.getFileInfo().locationPath();
        if (!locationPath.isEmpty()) {
            // Search directory and all sub-directories, i.e. for "path/to/dir"
            // also find files in "path/to/dir/subdir" but not in "path/to/dir copy".
            DEBUG_ASSERT(!locationPath.endsWith(QChar('/')));
            const QString locationPathWithTerminator =
                    locationPath + QChar('/');
            const QString searchQuery =
                    QStringLiteral("location:") +
                    quoteSearchQueryText(locationPathWithTerminator);
            addTriggerSearchAction(
                    &addSeparatorBeforeNextAction,
                    searchQuery,
                    tr("Directory"),
                    locationPathWithTerminator);
        }
    }

    addSeparator();

    // Make the Search button a checkbox to simplify setting an icon via qss.
    // This is not possible with a QAction, and tedious with a QPushButton.
    // Use a custom QCheckBox with fixed hover behavior.
    auto pCheckBox = make_parented<WMenuCheckBox>(tr("&Search selected"), this);
    pCheckBox->setObjectName("SearchSelectedAction");
    m_pSearchAction = make_parented<QWidgetAction>(this);
    m_pSearchAction->setDefaultWidget(pCheckBox.get());
    addAction(m_pSearchAction.get());
    m_pSearchAction->setDisabled(true);

    // This is for Enter/Return key
    connect(m_pSearchAction.get(),
            &QAction::triggered,
            this,
            &WSearchRelatedTracksMenu::combineQueriesTriggerSearch);
    // This is for click and Space key
    connect(pCheckBox.get(),
            &QCheckBox::clicked,
            this,
            &WSearchRelatedTracksMenu::combineQueriesTriggerSearch);
}

bool WSearchRelatedTracksMenu::eventFilter(QObject* pObj, QEvent* e) {
    if (e->type() == QEvent::MouseButtonPress) {
        // Since we want tp provide a toggle function that allows to check multiple
        // criteria (ie. don't auto-close the menu on first click) we need to
        // figure the intended click target.
        // Simply checking whether the click is inside the indicator's rectangle
        // is not sufficient: the indicator's width & height is only about 60%
        // of the item's total height, so there's top/left/bottom margin that
        // would activate the action.
        // Let's simply check if the click's x position is in the label region.
        // If it is, trigger search ignoring other checked boxes. Else toggle it.
        QCheckBox* pBox = qobject_cast<QCheckBox*>(pObj);
        if (pBox) {
            auto* pStyle = pBox->style();
            QStyleOptionButton option;
            option.initFrom(pBox);
            const QRect labelRect = pStyle->subElementRect(QStyle::SE_CheckBoxContents,
                    &option,
                    pBox);
            QMouseEvent* pMe = static_cast<QMouseEvent*>(e);
            if (pMe->pos().x() > labelRect.left()) {
                // Label region was clicked, trigger the search.
                const QString query = pBox->property("query").toString();
                emit triggerSearch(query);
                // Note that this click will not emit QAction::triggered like
                // when pressing Return on a selected action, hence we need to
                // make sure WTrackMenu closes when receiving triggerSearch().
            } else {
                pBox->toggle();
            }
            return true;
        }
    }
    return QObject::eventFilter(pObj, e);
}

void WSearchRelatedTracksMenu::updateSearchButton() {
    // Enable the Search button if at least opChildbox is checked.
    VERIFY_OR_DEBUG_ASSERT(m_pSearchAction) {
        return;
    }
    m_pSearchAction->setDisabled(true);
    for (const auto* pChild : std::as_const(children())) {
        const auto* pBox = qobject_cast<const QCheckBox*>(pChild);
        if (pBox && pBox->isChecked()) {
            m_pSearchAction->setEnabled(true);
            return;
        }
    }
}

void WSearchRelatedTracksMenu::combineQueriesTriggerSearch() {
    // collect queries of all checked checkboxes
    QStringList queries;
    for (const auto* pChild : std::as_const(children())) {
        if (pChild == m_pSearchAction) {
            continue;
        }
        const auto* pBox = qobject_cast<const QCheckBox*>(pChild);
        if (pBox && pBox->isChecked()) {
            QString query = pBox->property("query").toString();
            if (!query.isEmpty()) {
                queries.append(query);
            }
        }
    }
    if (queries.isEmpty()) {
        return;
    } else {
        QString queryCombo = queries.join(QChar(' '));
        emit triggerSearch(queryCombo);
    }
}

void WSearchRelatedCheckBox::paintEvent(QPaintEvent*) {
    // start original QCheckBox implementation
    QStylePainter painter(this);
    QStyleOptionButton opt;
    initStyleOption(&opt);
    painter.drawControl(QStyle::CE_CheckBox, opt);
    // end

    // Draw a vertical bar over the entire height at the left edge of the label
    const QStyle* pStyle = style();
    const QRect labelRect = pStyle->subElementRect(QStyle::SE_CheckBoxContents,
            &opt,
            this);
    const QRect frameRect = opt.rect;
    const QPoint top(labelRect.left(), frameRect.top());
    const QPoint bottom(labelRect.left(), frameRect.bottom());
    // We draw with the separator color from qss or, if that's not set,
    // with the palette's inactive text color.
    const QPen linePen(
            m_separatorColor.isValid() ? m_separatorColor
                                       : opt.palette.color(QPalette::Disabled, QPalette::Text),
            1,
            Qt::SolidLine,
            Qt::SquareCap);
    painter.setPen(linePen);
    painter.drawLine(top, bottom);
}
