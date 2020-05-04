#include <QFont>
#include <QShortcut>
#include <QStyle>

#include "wsearchlineedit.h"
#include "wskincolor.h"
#include "wwidget.h"

#include "skin/skincontext.h"

#include "util/assert.h"
#include "util/logger.h"

#define ENABLE_TRACE_LOG false

namespace {

const mixxx::Logger kLogger("WSearchLineEdit");

const QColor kDefaultForegroundColor = QColor(0, 0, 0);
const QColor kDefaultBackgroundColor = QColor(255, 255, 255);

const QString kEmptySearch = QStringLiteral("");

const QString kDisabledText = QStringLiteral("- - -");

const QString kClearButtonName = QStringLiteral("SearchClearButton");

inline QString clearButtonStyleSheet(int pxPaddingRight) {
    DEBUG_ASSERT(pxPaddingRight >= 0);
    return QString(
            QStringLiteral("QLineEdit { padding-right: %1px; }"))
            .arg(pxPaddingRight);
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

#if ENABLE_TRACE_LOG
QDebug operator<<(QDebug dbg, WSearchLineEdit::State state) {
    switch (state) {
    case WSearchLineEdit::State::Active:
        return dbg << "Active";
    case WSearchLineEdit::State::Inactive:
        return dbg << "Inactive";
    DEBUG_ASSERT(!"unreachable");
    return dbg;
}
#endif // ENABLE_TRACE_LOG

} // namespace

//static
constexpr int WSearchLineEdit::kMinDebouncingTimeoutMillis;

//static
constexpr int WSearchLineEdit::kDefaultDebouncingTimeoutMillis;

//static
constexpr int WSearchLineEdit::kMaxDebouncingTimeoutMillis;

//static
int WSearchLineEdit::s_debouncingTimeoutMillis = kDefaultDebouncingTimeoutMillis;

//static
void WSearchLineEdit::setDebouncingTimeoutMillis(int debouncingTimeoutMillis) {
    s_debouncingTimeoutMillis = verifyDebouncingTimeoutMillis(debouncingTimeoutMillis);
}

WSearchLineEdit::WSearchLineEdit(QWidget* pParent)
    : QLineEdit(pParent),
      WBaseWidget(this),
      m_clearButton(new QToolButton(this)),
      m_foregroundColor(kDefaultForegroundColor),
      m_state(State::Inactive) {
    DEBUG_ASSERT(kEmptySearch.isEmpty());
    DEBUG_ASSERT(!kEmptySearch.isNull());

    setAcceptDrops(false);
    setPlaceholderText(tr("Search...", "noun"));

    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->setObjectName(kClearButtonName);
    // Assume the qss border is at least 1px wide
    m_frameWidth = 1;
    m_clearButton->hide();
    connect(m_clearButton,
            &QAbstractButton::clicked,
            this,
            &WSearchLineEdit::clearSearch);

    // This prevents the searchbox from being focused by Tab key (real or emulated)
    // so it is skipped when using the library controls 'MoveFocus[...]'
    // The Clear button can still be focused by Tab.
    setFocusPolicy(Qt::ClickFocus);
    QShortcut* setFocusShortcut = new QShortcut(QKeySequence(tr("Ctrl+F", "Search|Focus")), this);
    connect(setFocusShortcut,
            &QShortcut::activated,
            this,
            &WSearchLineEdit::setShortcutFocus);

    // Set up a timer to search after a few hundred milliseconds timeout.  This
    // stops us from thrashing the database if you type really fast.
    m_debouncingTimer.setSingleShot(true);
    connect(&m_debouncingTimer,
            &QTimer::timeout,
            this,
            &WSearchLineEdit::triggerSearch);
    connect(this,
            &QLineEdit::textChanged,
            this,
            &WSearchLineEdit::updateText);

    // When you hit enter, it will trigger the search.
    connect(this,
            &QLineEdit::returnPressed,
            this,
            [this] {
                if (clearBtnHasFocus()) {
                    clearSearch();
                } else {
                    triggerSearch();
                }
            });

    QSize clearButtonSize = m_clearButton->sizeHint();

    // Ensures the text does not obscure the clear image.
    setStyleSheet(clearButtonStyleSheet(clearButtonSize.width() + m_frameWidth + 1));

    showPlaceholder();
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
            QColor(
                    255 - backgroundColor.red(),
                    255 - backgroundColor.green(),
                    255 - backgroundColor.blue());
    m_foregroundColor = defaultForegroundColor;
    QString fgColorName;
    if (context.hasNodeSelectString(node, "FgColor", &fgColorName)) {
        auto namedColor = QColor(fgColorName);
        if (namedColor.isValid()) {
            m_foregroundColor = namedColor;
        } else {
            kLogger.warning()
                    << "Failed to parse foreground color"
                    << fgColorName;
        }
    }
    m_foregroundColor = WSkinColor::getCorrectColor(m_foregroundColor);
    VERIFY_OR_DEBUG_ASSERT(m_foregroundColor != backgroundColor) {
        kLogger.warning()
                << "Invisible foreground color - using default color as fallback";
        m_foregroundColor = defaultForegroundColor;
    }
    kLogger.debug()
            << "Foreground color:"
            << m_foregroundColor;

    QPalette pal = palette();
    DEBUG_ASSERT(backgroundColor != m_foregroundColor);
    pal.setBrush(backgroundRole(), backgroundColor);
    pal.setBrush(foregroundRole(), m_foregroundColor);
    setPalette(pal);

    m_clearButton->setToolTip(tr("Clear input") + "\n" +
            tr("Clear the search bar input field") + "\n\n" +

            tr("Shortcut") + ": \n" +
            tr("Ctrl+Backspace"));

    setToolTip(tr("Search", "noun") + "\n" +
            tr("Enter a string to search for") + "\n" +
            tr("Use operators like bpm:115-128, artist:BooFar, -year:1990") + "\n" +
            tr("For more information see User Manual > Mixxx Library") + "\n\n" +

            tr("Shortcut") + ": \n" +
            tr("Ctrl+F") + "  " + tr("Focus", "Give search bar input focus") + "\n" +
            tr("Ctrl+Backspace") + "  " + tr("Clear input", "Clear the search bar input field") + "\n" +
            tr("Esc") + "  " + tr("Exit search", "Exit search bar and leave focus"));
}

void WSearchLineEdit::resizeEvent(QResizeEvent* e) {
    QLineEdit::resizeEvent(e);
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
        refreshEditBox();
    }
    int top = rect().top() + m_frameWidth;
    if (layoutDirection() == Qt::LeftToRight) {
        m_clearButton->move(rect().right() - m_innerHeight - m_frameWidth, top);
    } else {
        m_clearButton->move(m_frameWidth, top);
    }
}

QString WSearchLineEdit::getSearchText() const {
    if (isEnabled()) {
        DEBUG_ASSERT(!text().isNull());
        return text();
    } else {
        return QString();
    }
}

void WSearchLineEdit::switchState(State state) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "switchState"
            << m_state
            << "->"
            << state;
#endif // ENABLE_TRACE_LOG
    DEBUG_ASSERT(isEnabled());
    if (m_state == state) {
        // Nothing to do
        return;
    }
    QString text;
    switch (m_state) {
    case State::Active:
        // Active -> Inactive
        // Save the current search text BEFORE switching the state!
        text = getSearchText();
        m_state = State::Inactive;
        break;
    case State::Inactive:
        // Inactive -> Active
        // Get the current search text AFTER switching the state!
        m_state = State::Active;
        text = getSearchText();
        break;
    }
    updateEditBox(text);
}

void WSearchLineEdit::focusInEvent(QFocusEvent* event) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "focusInEvent";
#endif // ENABLE_TRACE_LOG
    QLineEdit::focusInEvent(event);
    switchState(State::Active);
}

void WSearchLineEdit::focusOutEvent(QFocusEvent* event) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "focusOutEvent";
#endif // ENABLE_TRACE_LOG
    QLineEdit::focusOutEvent(event);
    if (m_debouncingTimer.isActive()) {
        // Trigger a pending search before leaving the edit box.
        // Otherwise the entered text might be ignored and get lost
        // due to the debouncing timeout!
        triggerSearch();
    }
    switchState(State::Inactive);
    if (getSearchText().isEmpty()) {
        showPlaceholder();
    }
}

void WSearchLineEdit::setTextBlockSignals(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "setTextBlockSignals"
            << text;
#endif // ENABLE_TRACE_LOG
    blockSignals(true);
    setText(text);
    blockSignals(false);
}

// slot
void WSearchLineEdit::disableSearch() {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "disableSearch";
#endif // ENABLE_TRACE_LOG
    if (!isEnabled()) {
        return;
    }
    setTextBlockSignals(kDisabledText);
    // Set disabled AFTER switching the state!
    setEnabled(false);
}

// slot
void WSearchLineEdit::enableSearch(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "enableSearch"
            << text;
#endif // ENABLE_TRACE_LOG
    if (isEnabled()) {
        return;
    }
    // Set enabled BEFORE updating the edit box!
    setEnabled(true);
    updateEditBox(text);
}

// slot
void WSearchLineEdit::restoreSearch(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "restoreSearch"
            << text;
#endif // ENABLE_TRACE_LOG
    if (text.isNull()) {
        disableSearch();
    } else {
        enableSearch(text);
    }
}

// slot
void WSearchLineEdit::triggerSearch() {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "triggerSearch"
            << getSearchText();
#endif // ENABLE_TRACE_LOG
    DEBUG_ASSERT(isEnabled());
    m_debouncingTimer.stop();
    emit search(getSearchText());
}

bool WSearchLineEdit::shouldShowPlaceholder(const QString& text) const {
    DEBUG_ASSERT(isEnabled());
    return text.isEmpty() && m_state != State::Active;
}

void WSearchLineEdit::showPlaceholder() {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "showPlaceholder"
            << getSearchText();
#endif // ENABLE_TRACE_LOG
    DEBUG_ASSERT(getSearchText() == kEmptySearch);
    DEBUG_ASSERT(shouldShowPlaceholder(getSearchText()));

    updateClearButton(kEmptySearch);

    QPalette pal = palette();
    pal.setColor(foregroundRole(), Qt::lightGray);
    setPalette(pal);
}

void WSearchLineEdit::showSearchText(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "showSearchText"
            << text;
#endif // ENABLE_TRACE_LOG
    DEBUG_ASSERT(isEnabled());

    if (text.isEmpty()) {
        setTextBlockSignals(kEmptySearch);
    } else {
        setTextBlockSignals(text);
    }
    m_state = State::Active;
    updateClearButton(text);

    QPalette pal = palette();
    pal.setColor(foregroundRole(), m_foregroundColor);
    setPalette(pal);

    // This gets rid of the blue mac highlight.
    setAttribute(Qt::WA_MacShowFocusRect, false);
}

void WSearchLineEdit::refreshEditBox() {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "refreshEditBox";
#endif // ENABLE_TRACE_LOG
    if (isEnabled()) {
        enableSearch(getSearchText());
    } else {
        disableSearch();
    }
}

void WSearchLineEdit::updateEditBox(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "updateEditBox"
            << text;
#endif // ENABLE_TRACE_LOG
    // Updating the placeholder or search text implicitly updates
    // both the text and the clear button
    if (shouldShowPlaceholder(text)) {
        showPlaceholder();
    } else {
        showSearchText(text);
    }
}

void WSearchLineEdit::updateClearButton(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "updateClearButton"
            << text;
#endif // ENABLE_TRACE_LOG
    if (shouldShowPlaceholder(text)) {
        // Disable while placeholder is shown
        m_clearButton->setVisible(false);
        // no right padding
        setStyleSheet(clearButtonStyleSheet(0));
    } else {
        // Enable otherwise
        m_clearButton->setVisible(true);
        // make sure the text won't be drawn behind the Clear button icon
        setStyleSheet(clearButtonStyleSheet(m_innerHeight + m_frameWidth));
    }
}

bool WSearchLineEdit::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QLineEdit::event(pEvent);
}

// slot
bool WSearchLineEdit::clearBtnHasFocus() const {
    return m_clearButton->hasFocus();
}

// slot
void WSearchLineEdit::clearSearch() {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "clearSearch";
#endif // ENABLE_TRACE_LOG
    DEBUG_ASSERT(isEnabled());
    // Clearing the edit field will engage the debouncing timer
    // and gives the user the chance for entering a new search
    // before returning the whole (and probably huge) library.
    // No need to manually trigger a search at this point!
    // See also: https://bugs.launchpad.net/mixxx/+bug/1635087
    setText(kEmptySearch);
    // Refocus the edit field
    setFocus(Qt::OtherFocusReason);
}

// slot
void WSearchLineEdit::updateText(const QString& text) {
#if ENABLE_TRACE_LOG
    kLogger.trace()
            << "updateText"
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
}

// slot
void WSearchLineEdit::setShortcutFocus() {
    setFocus(Qt::ShortcutFocusReason);
}

// Use the same font as the library table and the sidebar
void WSearchLineEdit::slotSetFont(const QFont& font) {
    setFont(font);
}
