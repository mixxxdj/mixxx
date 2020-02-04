#include <QFont>
#include <QShortcut>
#include <QStyle>

#include "wsearchlineedit.h"
#include "wskincolor.h"
#include "wwidget.h"

#include "skin/skincontext.h"

#include "util/assert.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("WSearchLineEdit");

const QColor kDefaultForegroundColor = QColor(0, 0, 0);
const QColor kDefaultBackgroundColor = QColor(255, 255, 255);

const QString kEmptySearch = "";

const QString kDisabledText = "- - -";

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

    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->setToolTip(tr("Clear input", "Clear the search bar input field"));
    m_clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_clearButton->hide();
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clearSearch()));

    setFocusPolicy(Qt::ClickFocus);
    QShortcut* setFocusShortcut = new QShortcut(QKeySequence(tr("Ctrl+F", "Search|Focus")), this);
    connect(setFocusShortcut, SIGNAL(activated()), this, SLOT(setShortcutFocus()));

    // Set up a timer to search after a few hundred milliseconds timeout.  This
    // stops us from thrashing the database if you type really fast.
    m_debouncingTimer.setSingleShot(true);
    connect(&m_debouncingTimer, SIGNAL(timeout()), this, SLOT(triggerSearch()));

    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateText(const QString&)));

    // When you hit enter, it will trigger the search.
    connect(this, SIGNAL(returnPressed()), this, SLOT(triggerSearch()));

    // The width of the frame for the widget based on the styling.
    const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    // Ensures the text does not obscure the clear image.
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ")
                          .arg(m_clearButton->sizeHint().width() + frameWidth + 1));

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
}

void WSearchLineEdit::resizeEvent(QResizeEvent* e) {
    QLineEdit::resizeEvent(e);
    QSize sz = m_clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int height = (rect().bottom() + 1 - sz.height()) / 2;
    if (layoutDirection() == Qt::LeftToRight) {
        m_clearButton->move(rect().right() - frameWidth - sz.width() - 1, height);
    } else {
        m_clearButton->move(frameWidth + 1, height);
    }
}

QString WSearchLineEdit::getSearchText() const {
    if (isEnabled() && (m_state == State::Active)) {
        return text();
    } else {
        return QString();
    }
}

void WSearchLineEdit::focusInEvent(QFocusEvent* event) {
    QLineEdit::focusInEvent(event);
    showSearchText(getSearchText());
}

void WSearchLineEdit::focusOutEvent(QFocusEvent* event) {
    QLineEdit::focusOutEvent(event);
    updateEditBox(getSearchText());
}

// slot
void WSearchLineEdit::disableSearch() {
    restoreSearch(QString());
}

// slot
void WSearchLineEdit::restoreSearch(const QString& text) {
    if (text.isNull()) {
        // disable
        setEnabled(false);
        setText(kDisabledText);
    } else {
        setEnabled(true);
        // Updating the placeholder implicitly updates the text and the clear button
        updateEditBox(text);
    }
}

// slot
void WSearchLineEdit::triggerSearch() {
    m_debouncingTimer.stop();
    emit search(getSearchText());
}

void WSearchLineEdit::showPlaceholder() {
    DEBUG_ASSERT(isEnabled());

    // Deactivate text change listener
    m_state = State::Inactive;

    setText(tr("Search...", "noun"));
    setToolTip(tr("Search", "noun") + "\n" +
            tr("Enter a string to search for") + "\n" +
            tr("Use operators like bpm:115-128, artist:BooFar, -year:1990") + "\n" +
            tr("For more information see User Manual > Mixxx Library") + "\n\n" +

            tr("Shortcut") + ": \n" +
            tr("Ctrl+F") + "  " + tr("Focus", "Give search bar input focus") + "\n" +
            tr("Ctrl+Backspace") + "  " + tr("Clear input", "Clear the search bar input field") + "\n" +
            tr("Esc") + "  " + tr("Exit search", "Exit search bar and leave focus"));

    QPalette pal = palette();
    pal.setColor(foregroundRole(), Qt::lightGray);
    setPalette(pal);
}

void WSearchLineEdit::showSearchText(const QString& text) {
    DEBUG_ASSERT(isEnabled());

    // Reactivate text change listener
    m_state = State::Active;

    // Update the displayed text without (re-)starting the timer
    blockSignals(true);
    if (text.isNull()) {
        setText(kEmptySearch);
    } else {
        setText(text);
    }
    blockSignals(false);

    updateClearButton(text);

    QPalette pal = palette();
    pal.setColor(foregroundRole(), m_foregroundColor);
    setPalette(pal);

    // This gets rid of the blue mac highlight.
    setAttribute(Qt::WA_MacShowFocusRect, false);
}

void WSearchLineEdit::updateEditBox(const QString& text) {
    if (text.isEmpty()) {
        showPlaceholder();
    } else {
        showSearchText(text);
    }
}

void WSearchLineEdit::updateClearButton(const QString& text) {
    m_clearButton->setVisible(!text.isEmpty() && (m_state == State::Active));
}

bool WSearchLineEdit::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QLineEdit::event(pEvent);
}

// slot
void WSearchLineEdit::clearSearch() {
    DEBUG_ASSERT(m_state == State::Active);
    setText(kEmptySearch);
    // Clearing the edit field will engage the debouncing timer
    // and gives the user the chance for entering a new search
    // before returning the whole (and probably huge) library.
    // No need to manually trigger a search at this point!
    // See also: https://bugs.launchpad.net/mixxx/+bug/1635087
}

// slot
void WSearchLineEdit::updateText(const QString& text) {
    if (isEnabled() && (m_state == State::Active)) {
        updateClearButton(text);
        DEBUG_ASSERT(m_debouncingTimer.isSingleShot());
        if (s_debouncingTimeoutMillis > 0) {
            m_debouncingTimer.start(s_debouncingTimeoutMillis);
        } else {
            // Deactivate the timer if the timeout is invalid.
            // Disabling the timer permanently by setting the timeout
            // to an invalid value is an expected and valid use case.
            m_debouncingTimer.stop();
        }
    } else {
        updateClearButton(QString());
        m_debouncingTimer.stop();
    }
}

// slot
void WSearchLineEdit::setShortcutFocus() {
    setFocus(Qt::ShortcutFocusReason);
}
