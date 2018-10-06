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
      m_showingPlaceholder(false) {
    setAcceptDrops(false);

    QPixmap pixmap(":/images/library/cross.png");
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
    if (m_showingPlaceholder || !isEnabled()) {
        return QString();
    } else {
        return text();
    }
}

void WSearchLineEdit::focusInEvent(QFocusEvent* event) {
    QLineEdit::focusInEvent(event);
    hidePlaceholder(getSearchText());
}

void WSearchLineEdit::focusOutEvent(QFocusEvent* event) {
    QLineEdit::focusOutEvent(event);
    updatePlaceholder(getSearchText());
}

// slot
void WSearchLineEdit::disableSearch() {
    restoreSearch(QString());
}

// slot
void WSearchLineEdit::restoreSearch(const QString& text) {
    if (text.isNull()) {
        // disable
        blockSignals(true);
        setText(kDisabledText);
        blockSignals(false);
        setEnabled(false);
    } else {
        blockSignals(true);
        setText(text);
        blockSignals(false);
        updatePlaceholder(text);
        updateClearButton(text);
        setEnabled(true);
    }
}

// slot
void WSearchLineEdit::triggerSearch() {
    m_debouncingTimer.stop();
    emit search(getSearchText());
}

void WSearchLineEdit::showPlaceholder() {
    m_debouncingTimer.stop();

    // Must block signals here so that we don't emit a search() signal via
    // textChanged().
    blockSignals(true);
    setText(tr("Search...", "noun"));
    setToolTip(
            tr("Search", "noun") + "\n" + tr("Enter a string to search for") + "\n\n" +
            tr("Shortcut") + ": \n" + tr("Ctrl+F") + "  " +
            tr("Focus", "Give search bar input focus") + "\n" + tr("Ctrl+Backspace") + "  " +
            tr("Clear input", "Clear the search bar input field") + "\n" + tr("Esc") + "  " +
            tr("Exit search", "Exit search bar and leave focus"));
    m_showingPlaceholder = true;
    blockSignals(false);

    QPalette pal = palette();
    pal.setColor(foregroundRole(), Qt::lightGray);
    setPalette(pal);
}

void WSearchLineEdit::hidePlaceholder(const QString& text) {
    if (!m_showingPlaceholder) {
        return; // nothing to do
    }

    // Must block signals here so that we don't emit a search() signal via
    // textChanged().
    blockSignals(true);
    if (text.isNull()) {
        setText(kEmptySearch);
    } else {
        setText(text);
    }
    m_showingPlaceholder = false;
    blockSignals(false);

    QPalette pal = palette();
    pal.setColor(foregroundRole(), m_foregroundColor);
    setPalette(pal);

    // This gets rid of the blue mac highlight.
    setAttribute(Qt::WA_MacShowFocusRect, false);
}

void WSearchLineEdit::updatePlaceholder(const QString& text) {
    if (text.isEmpty()) {
        showPlaceholder();
    } else {
        hidePlaceholder(text);
    }
}

void WSearchLineEdit::updateClearButton(const QString& text) {
    m_clearButton->setVisible(!text.isEmpty() && !m_showingPlaceholder);
}

bool WSearchLineEdit::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QLineEdit::event(pEvent);
}

// slot
void WSearchLineEdit::clearSearch() {
    DEBUG_ASSERT(!m_showingPlaceholder);
    setText(kEmptySearch);
    // Enforce immediate update of track table
    triggerSearch();
}

// slot
void WSearchLineEdit::updateText(const QString& text) {
    if (m_showingPlaceholder) {
        DEBUG_ASSERT(!m_debouncingTimer.isActive());
        return; // do nothing while showing placeholder
    }

    if (text.isEmpty()) {
        m_debouncingTimer.stop();
        emit searchCleared();
    } else {
        DEBUG_ASSERT(m_debouncingTimer.isSingleShot());
        m_debouncingTimer.start(kDebouncingTimeoutMillis);
    }

    updateClearButton(text);
}

// slot
void WSearchLineEdit::setShortcutFocus() {
    setFocus(Qt::ShortcutFocusReason);
}
