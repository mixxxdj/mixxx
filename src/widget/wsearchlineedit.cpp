#include <QFont>
#include <QShortcut>
#include <QStyle>

#include "wsearchlineedit.h"
#include "wskincolor.h"
#include "wwidget.h"

#include "skin/skincontext.h"

#include "util/assert.h"

namespace {

// Delay for triggering a search while typing.
const int kDebouncingTimeoutMillis = 300;

const QString kEmptySearch = "";

const QString kDisabledText = "- - -";

} // namespace

WSearchLineEdit::WSearchLineEdit(QWidget* pParent)
    : QLineEdit(pParent),
      WBaseWidget(this),
      m_clearButton(new QToolButton(this)),
      m_state(State::Inactive) {
    DEBUG_ASSERT(kEmptySearch.isEmpty());
    DEBUG_ASSERT(!kEmptySearch.isNull());

    setAcceptDrops(false);

    QPixmap pixmap(":/images/library/ic_library_cross.svg");
    m_clearButton->setIcon(QIcon(pixmap));
    m_clearButton->setIconSize(pixmap.size());
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
    QColor backgroundColor(255, 255, 255);
    QString bgColorStr;
    if (context.hasNodeSelectString(node, "BgColor", &bgColorStr)) {
        backgroundColor.setNamedColor(bgColorStr);
        setAutoFillBackground(true);
    }
    QPalette pal = palette();
    pal.setBrush(backgroundRole(), WSkinColor::getCorrectColor(backgroundColor));

    m_foregroundColor = QColor(0, 0, 0);
    QString fgColorStr;
    if (context.hasNodeSelectString(node, "FgColor", &fgColorStr)) {
        m_foregroundColor.setNamedColor(fgColorStr);
    }
    backgroundColor = WSkinColor::getCorrectColor(backgroundColor);
    m_foregroundColor =
            QColor(255 - backgroundColor.red(), 255 - backgroundColor.green(),
                   255 - backgroundColor.blue());
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
    setToolTip(
            tr("Search", "noun") + "\n" + tr("Enter a string to search for") + "\n\n" +
            tr("Shortcut") + ": \n" + tr("Ctrl+F") + "  " +
            tr("Focus", "Give search bar input focus") + "\n" + tr("Ctrl+Backspace") + "  " +
            tr("Clear input", "Clear the search bar input field") + "\n" + tr("Esc") + "  " +
            tr("Exit search", "Exit search bar and leave focus"));

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
    // Enforce immediate update of track table
    triggerSearch();
}

// slot
void WSearchLineEdit::updateText(const QString& text) {
    if (isEnabled() && (m_state == State::Active)) {
        updateClearButton(text);
        DEBUG_ASSERT(m_debouncingTimer.isSingleShot());
        m_debouncingTimer.start(kDebouncingTimeoutMillis);
    } else {
        updateClearButton(QString());
        m_debouncingTimer.stop();
    }
}

// slot
void WSearchLineEdit::setShortcutFocus() {
    setFocus(Qt::ShortcutFocusReason);
}
