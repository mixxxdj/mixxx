#include "wsearchlineedit.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QFont>
#include <QLineEdit>
#include <QShortcut>
#include <QSizePolicy>
#include <QStyle>

#include "moc_wsearchlineedit.cpp"
#include "skin/legacy/skincontext.h"
#include "util/assert.h"
#include "util/logger.h"
#include "wskincolor.h"
#include "wwidget.h"

#define ENABLE_TRACE_LOG false

namespace {

const mixxx::Logger kLogger("WSearchLineEdit");

const QColor kDefaultBackgroundColor = QColor(0, 0, 0);

const QString kDisabledText = QStringLiteral("- - -");

const QString kSavedQueriesConfigGroup = QStringLiteral("[SearchQueries]");

// Border width, max. 2 px when focused (in official skins)
constexpr int kBorderWidth = 2;

int verifyDebouncingTimeoutMillis(int debouncingTimeoutMillis) {
    VERIFY_OR_DEBUG_ASSERT(debouncingTimeoutMillis >= WSearchLineEdit::kMinDebouncingTimeoutMillis) {
        debouncingTimeoutMillis = WSearchLineEdit::kMinDebouncingTimeoutMillis;
    }
    VERIFY_OR_DEBUG_ASSERT(debouncingTimeoutMillis <= WSearchLineEdit::kMaxDebouncingTimeoutMillis) {
        debouncingTimeoutMillis = WSearchLineEdit::kMaxDebouncingTimeoutMillis;
    }
    return debouncingTimeoutMillis;
}

} // namespace

//static
constexpr int WSearchLineEdit::kMinDebouncingTimeoutMillis;

//static
constexpr int WSearchLineEdit::kDefaultDebouncingTimeoutMillis;

//static
constexpr int WSearchLineEdit::kMaxDebouncingTimeoutMillis;

//static
constexpr int WSearchLineEdit::kSaveTimeoutMillis;

//static
constexpr int WSearchLineEdit::kMaxSearchEntries;

//static
int WSearchLineEdit::s_debouncingTimeoutMillis = kDefaultDebouncingTimeoutMillis;

//static
void WSearchLineEdit::setDebouncingTimeoutMillis(int debouncingTimeoutMillis) {
    s_debouncingTimeoutMillis = verifyDebouncingTimeoutMillis(debouncingTimeoutMillis);
}

WSearchLineEdit::WSearchLineEdit(QWidget* pParent, UserSettingsPointer pConfig)
        : QComboBox(pParent),
          WBaseWidget(this),
          m_pConfig(pConfig),
          m_clearButton(make_parented<QToolButton>(this)) {
    qRegisterMetaType<FocusWidget>("FocusWidget");
    setAcceptDrops(false);
    setEditable(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setIconSize(QSize(0, 0));
    setInsertPolicy(QComboBox::InsertAtTop);
    setMinimumSize(0, 0);
    setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToMinimumContentsLengthWithIcon);

    //: Shown in the library search bar when it is empty.
    lineEdit()->setPlaceholderText(tr("Search..."));

    // The goal is to make Esc natively close the popup, while in the line edit it
    // should move the keyboard focus to the tracks table. Unfortunately, eventFilter()
    // can't catch Esc before the popup is closed, and keyPressEvent() can't catch
    // keyPresses sent to the popup. So the only way to get this to work is to use
    // keyPressEvent() for catching all keypress events sent to the line edit,
    // while eventFilter() catches those sent to the popup.
    view()->installEventFilter(this);

    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->setObjectName(QStringLiteral("SearchClearButton"));

    m_clearButton->hide();
    connect(m_clearButton,
            &QAbstractButton::clicked,
            this,
            &WSearchLineEdit::slotClearSearch);

    QShortcut* setFocusShortcut = new QShortcut(QKeySequence(tr("Ctrl+F", "Search|Focus")), this);
    connect(setFocusShortcut,
            &QShortcut::activated,
            this,
            &WSearchLineEdit::slotSetShortcutFocus);

    // Set up a timer to search after a few hundred milliseconds timeout.  This
    // stops us from thrashing the database if you type really fast.
    m_debouncingTimer.setSingleShot(true);
    m_saveTimer.setSingleShot(true);
    connect(&m_debouncingTimer,
            &QTimer::timeout,
            this,
            &WSearchLineEdit::slotTriggerSearch);
    connect(&m_saveTimer,
            &QTimer::timeout,
            this,
            &WSearchLineEdit::slotSaveSearch);
    connect(this,
            &QComboBox::currentTextChanged,
            this,
            &WSearchLineEdit::slotTextChanged);
    connect(this,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &WSearchLineEdit::slotIndexChanged);

    loadQueriesFromConfig();

    refreshState();
}

WSearchLineEdit::~WSearchLineEdit() {
    saveQueriesInConfig();
}

void WSearchLineEdit::setup(const QDomNode& node, const SkinContext& context) {
    auto backgroundColor = kDefaultBackgroundColor;
    QString bgColorName;
    if (context.hasNodeSelectString(node, "BgColor", &bgColorName)) {
        auto namedColor = QColor(bgColorName);
        if (namedColor.isValid()) {
            backgroundColor = namedColor;
            setAutoFillBackground(true);
        } else {
            kLogger.warning()
                    << "Failed to parse background color"
                    << bgColorName;
        }
    }
    backgroundColor = WSkinColor::getCorrectColor(backgroundColor);
    kLogger.debug()
            << "Background color:"
            << backgroundColor;

    const auto defaultForegroundColor =
            QColor::fromRgb(
                    255 - backgroundColor.red(),
                    255 - backgroundColor.green(),
                    255 - backgroundColor.blue());
    auto foregroundColor = defaultForegroundColor;
    QString fgColorName;
    if (context.hasNodeSelectString(node, "FgColor", &fgColorName)) {
        auto namedColor = QColor(fgColorName);
        if (namedColor.isValid()) {
            foregroundColor = namedColor;
        } else {
            kLogger.warning()
                    << "Failed to parse foreground color"
                    << fgColorName;
        }
    }
    foregroundColor = WSkinColor::getCorrectColor(foregroundColor);
    VERIFY_OR_DEBUG_ASSERT(foregroundColor != backgroundColor) {
        kLogger.warning()
                << "Invisible foreground color - using default color as fallback";
        foregroundColor = defaultForegroundColor;
    }
    //kLogger.debug()
    //        << "Foreground color:"
    //        << foregroundColor;

    QPalette pal = palette();
    DEBUG_ASSERT(backgroundColor != foregroundColor);
    pal.setBrush(backgroundRole(), backgroundColor);
    pal.setBrush(foregroundRole(), foregroundColor);
    auto placeholderColor = foregroundColor;
    placeholderColor.setAlpha(placeholderColor.alpha() * 3 / 4); // 75% opaque
    //kLogger.debug()
    //        << "Placeholder color:"
    //        << placeholderColor;
    pal.setBrush(QPalette::PlaceholderText, placeholderColor);
    setPalette(pal);

    m_clearButton->setToolTip(tr("Clear input") + "\n" +
            tr("Clear the search bar input field") + "\n\n" +

            tr("Shortcut") + ": \n" +
            tr("Ctrl+Backspace"));

    setBaseTooltip(tr("Search", "noun") + "\n" +
            tr("Enter a string to search for") + "\n" +
            tr("Use operators like bpm:115-128, artist:BooFar, -year:1990") +
            "\n" + tr("For more information see User Manual > Mixxx Library") +
            "\n\n" +
            tr("Shortcuts") + ": \n" +
            tr("Ctrl+F") + "  " +
            tr("Focus", "Give search bar input focus") + "\n" +
            tr("Ctrl+Backspace") + "  " +
            tr("Clear input", "Clear the search bar input field") + "\n" +
            tr("Ctrl+Space") + "  " +
            tr("Toggle search history",
                    "Shows/hides the search history entries") +
            "\n" +
            tr("Delete or Backspace") + "  " + tr("Delete query from history") + "\n" +
            tr("Esc") + "  " + tr("Exit search", "Exit search bar and leave focus"));
}

void WSearchLineEdit::loadQueriesFromConfig() {
    if (!m_pConfig) {
        return;
    }
    const QList<ConfigKey> queryKeys =
            m_pConfig->getKeysWithGroup(kSavedQueriesConfigGroup);
    QSet<QString> queryStrings;
    for (const auto& queryKey : queryKeys) {
        QString queryString = m_pConfig->getValueString(queryKey).trimmed();
        if (queryString.isEmpty() || queryStrings.contains(queryString)) {
            // Don't add duplicate and remove it from the config immediately
            m_pConfig->remove(queryKey);
        } else {
            // Restore query
            addItem(queryString);
            queryStrings.insert(queryString);
        }
    }
}

void WSearchLineEdit::saveQueriesInConfig() {
    if (!m_pConfig) {
        return;
    }
    // Delete saved queries in case the list was cleared
    const QList<ConfigKey> queryKeys =
            m_pConfig->getKeysWithGroup(kSavedQueriesConfigGroup);
    for (const auto& queryKey : queryKeys) {
        m_pConfig->remove(queryKey);
    }
    // Store queries
    for (int index = 0; index < count(); index++) {
        m_pConfig->setValue(
                ConfigKey(kSavedQueriesConfigGroup, QString::number(index)),
                itemText(index).trimmed());
    }
}

void WSearchLineEdit::resizeEvent(QResizeEvent* e) {
    QComboBox::resizeEvent(e);
    m_innerHeight = height() - 2 * kBorderWidth;
    // Test if this is a vertical resize due to changed library font.
    // Assuming current button height is innerHeight from last resize,
    // we will resize the Clear button icon only if height has changed.
    if (m_clearButton->size().height() != m_innerHeight) {
        QSize newSize = QSize(m_innerHeight, m_innerHeight);
        m_clearButton->resize(newSize);
        m_clearButton->setIconSize(newSize);
        // Needed to update the Clear button and the down arrow
        // after skin change/reload.
        refreshState();
    }
    int top = rect().top() + kBorderWidth;
    if (layoutDirection() == Qt::LeftToRight) {
        m_clearButton->move(rect().right() -
                        static_cast<int>(1.7 * m_innerHeight) - kBorderWidth,
                top);
    } else {
        m_clearButton->move(static_cast<int>(0.7 * m_innerHeight) + kBorderWidth,
                top);
    }
}

QString WSearchLineEdit::getSearchText() const {
    if (isEnabled()) {
        DEBUG_ASSERT(!currentText().isNull());
        return currentText();
    } else {
        return QString();
    }
}

bool WSearchLineEdit::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        const int key = keyEvent->key();
        // Esc has already closed the popup by now and we don't want to process it.
        // We don't need to handle Up/Down in the popup either.
        // Any other keypress is forwarded.
        if (key != Qt::Key_Escape &&
                key != Qt::Key_Down &&
                key != Qt::Key_Up) {
            keyPressEvent(keyEvent);
            return true;
        }
    }
    return QComboBox::eventFilter(obj, event);
}

void WSearchLineEdit::keyPressEvent(QKeyEvent* keyEvent) {
    int currentTextIndex = 0;
    switch (keyEvent->key()) {
    // Ctrl + F is handled in slotSetShortcutFocus()
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        // If the popup is open remove the highlighted item from the list
        if (view()->isVisible()) {
            deleteSelectedListItem();
            return;
        }
        break;
    case Qt::Key_Up:
        // If we're at the top of the list the Up key clears the search bar,
        // no matter if it's a saved or unsaved query.
        // Otherwise Up is handled by the combobox itself.
        currentTextIndex = findCurrentTextIndex();
        if (currentTextIndex == 0 ||
                (currentTextIndex == -1 && !currentText().isEmpty())) {
            slotClearSearch();
            return;
        }
        break;
    case Qt::Key_Down:
        // After clearing the text field the Down key
        // is expected to show the latest query
        if (currentText().isEmpty()) {
            setCurrentIndex(0);
            return;
        }
        // After entering a new search query the Down key saves the query,
        // then selects the previous query
        if (findCurrentTextIndex() == -1) {
            slotSaveSearch();
        }
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (slotClearSearchIfClearButtonHasFocus()) {
            return;
        }
        if (findCurrentTextIndex() == -1) {
            slotSaveSearch();
        }
        slotTriggerSearch();
        return;
    case Qt::Key_Space:
        // Open/close popup with Ctrl + space
        if (keyEvent->modifiers() == Qt::ControlModifier) {
            if (view()->isVisible()) {
                hidePopup();
            } else {
                showPopup();
            }
            return;
        }
        // Space in popup emulates Return press
        if (view()->isVisible()) {
            QKeyEvent returnPress(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
            QApplication::sendEvent(view(), &returnPress);
            return;
        }
        break;
    case Qt::Key_Escape:
        emit setLibraryFocus(FocusWidget::TracksTable);
        return;
    default:
        // Don't change the query while the popup is open. This would cause the
        // same weird vertical squeezing like removing the current index.
        if (view()->isVisible()) {
            return;
        }
        break;
    }

    return QComboBox::keyPressEvent(keyEvent);
}

void WSearchLineEdit::focusInEvent(QFocusEvent* event) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "focusInEvent";
#endif // ENABLE_TRACE_LOG
    QComboBox::focusInEvent(event);
    updateClearAndDropdownButton(currentText());
}

void WSearchLineEdit::focusOutEvent(QFocusEvent* event) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "focusOutEvent";
#endif // ENABLE_TRACE_LOG
    slotSaveSearch();
    QComboBox::focusOutEvent(event);
    if (m_debouncingTimer.isActive()) {
        // Trigger a pending search before leaving the edit box.
        // Otherwise the entered text might be ignored and get lost
        // due to the debouncing timeout!
        slotTriggerSearch();
    }
}

void WSearchLineEdit::setTextBlockSignals(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "setTextBlockSignals"
            << text;
#endif // ENABLE_TRACE_LOG
    blockSignals(true);
    setCurrentText(text);
    blockSignals(false);
}

void WSearchLineEdit::slotDisableSearch() {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "slotDisableSearch";
#endif // ENABLE_TRACE_LOG
    if (!isEnabled()) {
        return;
    }
    setTextBlockSignals(kDisabledText);
    setEnabled(false);
    updateClearAndDropdownButton(QString());
}

void WSearchLineEdit::enableSearch(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "enableSearch"
            << text;
#endif // ENABLE_TRACE_LOG
    // Set enabled BEFORE updating the edit box!
    setEnabled(true);
    updateEditBox(text);
}

void WSearchLineEdit::slotRestoreSearch(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "slotRestoreSearch"
            << text;
#endif // ENABLE_TRACE_LOG
    if (text.isNull()) {
        slotDisableSearch();
    } else {
        // we save the current search before we switch to a new text
        slotSaveSearch();
        enableSearch(text);
    }
}

void WSearchLineEdit::slotTriggerSearch() {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "slotTriggerSearch"
            << getSearchText();
#endif // ENABLE_TRACE_LOG
    DEBUG_ASSERT(isEnabled());
    m_debouncingTimer.stop();
    emit search(getSearchText());
}

/// saves the current query as selection
void WSearchLineEdit::slotSaveSearch() {
    m_saveTimer.stop();
    QString cText = currentText().trimmed();
    int cIndex = findCurrentTextIndex();
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "save search. Text:"
            << cText
            << "Index:"
            << cIndex;
#endif // ENABLE_TRACE_LOG
    if (cText.isEmpty() || !isEnabled()) {
        return;
    }
    if (cIndex == -1) {
        removeItem(-1);
    }

    // Check if the text is already listed
    QSet<QString> querySet;
    for (int index = 0; index < count(); index++) {
        querySet.insert(itemText(index));
    }
    if (querySet.contains(cText)) {
        // If query exists clear the box and use its index to set the currentIndex
        int cIndex = findData(cText, Qt::DisplayRole);
        setCurrentIndex(cIndex);
        return;
    } else {
        // Else add it at the top
        insertItem(0, cText);
        setCurrentIndex(0);
        while (count() > kMaxSearchEntries) {
            removeItem(kMaxSearchEntries);
        }
    }
}

void WSearchLineEdit::slotMoveSelectedHistory(int steps) {
    if (!isEnabled()) {
        return;
    }
    int nIndex = currentIndex() + steps;
    // at the top we manually wrap around to the last entry.
    // at the bottom wrap-around happens automatically due to invalid nIndex.
    if (nIndex < -1) {
        nIndex = count() - 1;
    }
    setCurrentIndex(nIndex);
    m_saveTimer.stop();
}

void WSearchLineEdit::slotDeleteCurrentItem() {
    if (!isEnabled()) {
        return;
    }
    if (view()->isVisible()) {
        deleteSelectedListItem();
    } else {
        deleteSelectedComboboxItem();
    }
}

void WSearchLineEdit::deleteSelectedComboboxItem() {
    int cIndex = currentIndex();
    if (cIndex == -1) {
        return;
    } else {
        slotClearSearch();
        QComboBox::removeItem(cIndex);
    }
}

void WSearchLineEdit::deleteSelectedListItem() {
    // When the active query is removed the combobox would pick a sibling and
    // trigger a search. Avoid that, just clear the search once.
    // Also, there is a style update bug when changing the text of the hidden
    // line edit, see updateClearAndDropdownButton()
    int currViewindex = view()->currentIndex().row();
    if (currViewindex == findCurrentTextIndex()) {
        slotClearSearch();
    }
    QComboBox::removeItem(currViewindex);
    // ToDo Resize the list to new item size

    // Close the popup if all items were removed
    if (count() == 0) {
        hidePopup();
    }
}

void WSearchLineEdit::refreshState() {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "refreshState";
#endif // ENABLE_TRACE_LOG
    if (isEnabled()) {
        enableSearch(getSearchText());
    } else {
        slotDisableSearch();
    }
}

void WSearchLineEdit::showPopup() {
    int cIndex = findCurrentTextIndex();
    if (cIndex == -1) {
        slotSaveSearch();
    } else {
        m_saveTimer.stop();
        setCurrentIndex(cIndex);
    }
    QComboBox::showPopup();
    // When (empty) index -1 is selected in the combobox and the list view is opened
    // index 0 is auto-set but not highlighted.
    // Select first index manually (only in the list).
    if (cIndex == -1) {
        view()->selectionModel()->select(
                view()->currentIndex(),
                QItemSelectionModel::Select);
    }
}

void WSearchLineEdit::updateEditBox(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "updateEditBox"
            << text;
#endif // ENABLE_TRACE_LOG
    DEBUG_ASSERT(isEnabled());

    if (text.isEmpty()) {
        setTextBlockSignals(QString());
    } else {
        setTextBlockSignals(text);
    }
    updateClearAndDropdownButton(text);

    // This gets rid of the blue mac highlight.
    setAttribute(Qt::WA_MacShowFocusRect, false);
}

void WSearchLineEdit::updateClearAndDropdownButton(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "updateClearAndDropdownButton"
            << text;
#endif // ENABLE_TRACE_LOG
    // If the popup is open there's no need to further adjust the style, this is
    // invoked by focusInEvent when the popup is closed.
    // NOTE(ronso0) Also, when changing the text programmatically while the popup
    // is open this style update would cause the QLineEdit to shrink in height
    // (looks like a repaint without the padding from external stylesheets,
    // noticed on Linux. Probably related)
    if (view()->isVisible()) {
        return;
    }

    // Hide clear button if the text is empty and while placeholder is shown,
    // see disableSearch()
    m_clearButton->setVisible(!text.isEmpty());

    // Ensure the text is not obscured by the clear button. Otherwise no text,
    // no clear button, so the placeholder should use the entire width.
    const int paddingPx = text.isEmpty() ? 0 : m_innerHeight;
    const QString clearPos(layoutDirection() == Qt::RightToLeft ? "left" : "right");

    // Hide the nonfunctional drop-down button (set width to 0) if the search is disabled.
    const int dropDownWidth = isEnabled() ? static_cast<int>(m_innerHeight * 0.7) : 0;

    const QString styleSheet = QStringLiteral(
            "WSearchLineEdit { padding-%1: %2px; }"
            // With every paintEvent(?) the width of the drop-down button
            // is reset to default, so we need to re-adjust it.
            "WSearchLineEdit::down-arrow,"
            "WSearchLineEdit::drop-down {"
            "subcontrol-origin: padding;"
            "subcontrol-position: %1 center;"
            "width: %3; height: %4;}")
                                       .arg(clearPos,
                                               QString::number(paddingPx),
                                               QString::number(dropDownWidth),
                                               QString::number(m_innerHeight));
    setStyleSheet(styleSheet);
}

bool WSearchLineEdit::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QComboBox::event(pEvent);
}

void WSearchLineEdit::slotClearSearch() {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "slotClearSearch";
#endif // ENABLE_TRACE_LOG
    if (!isEnabled()) {
        return;
    }
    // select the last entry as current before cleaning the text field
    // so arrow keys will work as expected
    setCurrentIndex(-1);
    // Clearing the edit field will engage the debouncing timer
    // and gives the user the chance for entering a new search
    // before returning the whole (and probably huge) library.
    // No need to manually trigger a search at this point!
    // See also: https://bugs.launchpad.net/mixxx/+bug/1635087
    // Note that just clear() would also erase all combobox items,
    // thus clear the entire search history.
    lineEdit()->clear();
    // Refocus the edit field
    setFocus(Qt::OtherFocusReason);
}

bool WSearchLineEdit::slotClearSearchIfClearButtonHasFocus() {
    if (!m_clearButton->hasFocus()) {
        return false;
    }
    slotClearSearch();
    return true;
}

void WSearchLineEdit::slotIndexChanged(int index) {
    if (index != -1) {
        m_saveTimer.stop();
    }
}

void WSearchLineEdit::slotTextChanged(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "slotTextChanged"
            << text;
#endif // ENABLE_TRACE_LOG
    m_debouncingTimer.stop();
    if (!isEnabled()) {
        setTextBlockSignals(kDisabledText);
        return;
    }
    updateClearAndDropdownButton(text);
    DEBUG_ASSERT(m_debouncingTimer.isSingleShot());
    if (s_debouncingTimeoutMillis > 0) {
        m_debouncingTimer.start(s_debouncingTimeoutMillis);
    } else {
        // Don't (re-)activate the timer if the timeout is invalid.
        // Disabling the timer permanently by setting the timeout
        // to an invalid value is an expected and valid use case.
        DEBUG_ASSERT(!m_debouncingTimer.isActive());
    }
    m_saveTimer.start(kSaveTimeoutMillis);
}

void WSearchLineEdit::slotSetShortcutFocus() {
    if (hasFocus()) {
        lineEdit()->selectAll();
    } else {
        setFocus(Qt::ShortcutFocusReason);
    }
}

// Use the same font as the library table and the sidebar
void WSearchLineEdit::slotSetFont(const QFont& font) {
    setFont(font);
    if (lineEdit()) {
        lineEdit()->setFont(font);
        // Decreasing the font doesn't trigger a resizeEvent,
        // so we immediately refresh the controls manually.
        updateClearAndDropdownButton(getSearchText());
    }
}
