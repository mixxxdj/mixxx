#include "wsearchlineedit.h"

#include <QAbstractItemView>
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

constexpr int kClearButtonClearence = 1;

inline QString clearButtonStyleSheet(int pxPadding, Qt::LayoutDirection direction) {
    DEBUG_ASSERT(pxPadding >= 0);
    if (direction == Qt::RightToLeft) {
        return QString(
                QStringLiteral("WSearchLineEdit { padding-left: %1px; }"))
                .arg(pxPadding);
    } else {
        return QString(
                QStringLiteral("WSearchLineEdit { padding-right: %1px; }"))
                .arg(pxPadding);
    }
}

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

WSearchLineEdit::WSearchLineEdit(QWidget* pParent)
        : QComboBox(pParent),
          WBaseWidget(this),
          m_clearButton(make_parented<QToolButton>(this)) {
    setAcceptDrops(false);
    setEditable(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setIconSize(QSize(0, 0));
    setInsertPolicy(QComboBox::InsertAtTop);
    setMinimumSize(0, 0);
    setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToMinimumContentsLengthWithIcon);

    //: Shown in the library search bar when it is empty.
    lineEdit()->setPlaceholderText(tr("Search..."));
    installEventFilter(this);
    view()->installEventFilter(this);

    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->setObjectName(QStringLiteral("SearchClearButton"));
    // Query style for arrow width and frame border
    updateStyleMetrics();

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

    // When you hit enter, it will trigger or clear the search.
    connect(this->lineEdit(),
            &QLineEdit::returnPressed,
            this,
            [this] {
                if (!slotClearSearchIfClearButtonHasFocus()) {
                    slotTriggerSearch();
                }
            });

    QSize clearButtonSize = m_clearButton->sizeHint();

    // Ensures the text does not obscure the clear image.
    setStyleSheet(clearButtonStyleSheet(
            clearButtonSize.width() + m_frameWidth + kClearButtonClearence,
            layoutDirection()));

    refreshState();
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

    setToolTip(tr("Search", "noun") + "\n" +
            tr("Enter a string to search for") + "\n" +
            tr("Use operators like bpm:115-128, artist:BooFar, -year:1990") +
            "\n" + tr("For more information see User Manual > Mixxx Library") +
            "\n\n" +
            tr("Shortcut") + ": \n" + tr("Ctrl+F") + "  " +
            tr("Focus", "Give search bar input focus") + "\n" +
            tr("Ctrl+Backspace") + "  " +
            tr("Clear input", "Clear the search bar input field") + "\n" +
            tr("Ctrl+Space") + "  " +
            tr("Toggle search history",
                    "Shows/hides the search history entries") +
            "\n" +
            tr("Esc") + "  " + tr("Exit search", "Exit search bar and leave focus"));
}

void WSearchLineEdit::updateStyleMetrics() {
    QStyleOptionComboBox styleArrow;
    styleArrow.initFrom(this);
    QRect rectArrow(style()->subControlRect(
            QStyle::CC_ComboBox, &styleArrow, QStyle::SC_ComboBoxArrow, this));

    m_dropButtonWidth = rectArrow.width() + 1;
    m_frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, this);
}

void WSearchLineEdit::resizeEvent(QResizeEvent* e) {
    QComboBox::resizeEvent(e);
    updateStyleMetrics();
    m_innerHeight = this->height() - 2 * m_frameWidth;
    // Test if this is a vertical resize due to changed library font.
    // Assuming current button height is innerHeight from last resize,
    // we will resize the Clear button icon only if height has changed.
    if (m_clearButton->size().height() != m_innerHeight) {
        QSize newSize = QSize(m_innerHeight, m_innerHeight);
        m_clearButton->resize(newSize);
        m_clearButton->setIconSize(newSize);
        // Note(ronso0): For some reason this ensures the search text
        // is being displayed after skin change/reload.
        refreshState();
    }
    int top = rect().top() + m_frameWidth;
    if (layoutDirection() == Qt::LeftToRight) {
        m_clearButton->move(rect().right() - m_innerHeight - m_frameWidth - m_dropButtonWidth, top);
    } else {
        m_clearButton->move(m_frameWidth + m_dropButtonWidth, top);
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
        // if the popup is open don't intercept Up/Down keys
        if (!view()->isVisible()) {
            if (keyEvent->key() == Qt::Key_Up) {
                // if we're at the top of the list the Up key clears the search bar,
                // no matter if it's a saved and unsaved query
                if (findCurrentTextIndex() == 0 ||
                        (findCurrentTextIndex() == -1 && !currentText().isEmpty())) {
                    slotClearSearch();
                    return true;
                }
            } else if (keyEvent->key() == Qt::Key_Down) {
                // after clearing the text field the down key is expected to
                // show the latest entry
                if (currentText().isEmpty()) {
                    setCurrentIndex(0);
                    return true;
                }
                // in case the user entered a new search query
                // and presses the down key, save the query for later recall
                if (findCurrentTextIndex() == -1) {
                    slotSaveSearch();
                }
            }
        }
        if (keyEvent->key() == Qt::Key_Enter) {
            if (findCurrentTextIndex() == -1) {
                slotSaveSearch();
            }
            // The default handler will add the entry to the list,
            // this already happened in slotSaveSearch
            slotTriggerSearch();
            return true;
        } else if (keyEvent->key() == Qt::Key_Space &&
                keyEvent->modifiers() == Qt::ControlModifier) {
            // open/close popup on ctrl + space
            if (view()->isVisible()) {
                hidePopup();
            } else {
                showPopup();
            }
            return true;
        }
    }
    return QComboBox::eventFilter(obj, event);
}

void WSearchLineEdit::focusInEvent(QFocusEvent* event) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "focusInEvent";
#endif // ENABLE_TRACE_LOG
    QComboBox::focusInEvent(event);
}

void WSearchLineEdit::focusOutEvent(QFocusEvent* event) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "focusOutEvent";
#endif // ENABLE_TRACE_LOG
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
    int cIndex = findCurrentTextIndex();
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "save search. Index: "
            << cIndex;
#endif // ENABLE_TRACE_LOG
    m_saveTimer.stop();
    // entry already exists and is on top
    if (cIndex == 0) {
        return;
    }
    if (!currentText().isEmpty() && isEnabled()) {
        // we remove the existing item and add a new one at the top
        if (cIndex != -1) {
            removeItem(cIndex);
        }
        insertItem(0, currentText());
        setCurrentIndex(0);
        while (count() > kMaxSearchEntries) {
            removeItem(kMaxSearchEntries);
        }
    }
}

void WSearchLineEdit::slotMoveSelectedHistory(int steps) {
    int nIndex = currentIndex() + steps;
    // we wrap around to the last entry on backwards direction
    if (nIndex < -1) {
        nIndex = count() - 1;
    }
    setCurrentIndex(nIndex);
    m_saveTimer.stop();
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
    updateClearButton(text);

    // This gets rid of the blue mac highlight.
    setAttribute(Qt::WA_MacShowFocusRect, false);
}

void WSearchLineEdit::updateClearButton(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "updateClearButton"
            << text;
#endif // ENABLE_TRACE_LOG
    DEBUG_ASSERT(isEnabled());

    if (text.isEmpty()) {
        // Disable while placeholder is shown
        m_clearButton->setVisible(false);
        // no right padding
        setStyleSheet(clearButtonStyleSheet(0, layoutDirection()));
    } else {
        // Enable otherwise
        m_clearButton->setVisible(true);
        // make sure the text won't be drawn behind the Clear button icon
        setStyleSheet(clearButtonStyleSheet(
                m_innerHeight + m_dropButtonWidth +
                        m_frameWidth + kClearButtonClearence,
                layoutDirection()));
    }
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
    DEBUG_ASSERT(isEnabled());
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
    updateClearButton(text);
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
    setFocus(Qt::ShortcutFocusReason);
}

// Use the same font as the library table and the sidebar
void WSearchLineEdit::slotSetFont(const QFont& font) {
    updateStyleMetrics();
    setFont(font);
    if (lineEdit()) {
        lineEdit()->setFont(font);
    }
}
