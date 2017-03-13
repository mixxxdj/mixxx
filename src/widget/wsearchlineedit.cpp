#include "wwidget.h"
#include "wskincolor.h"
#include "wsearchlineedit.h"

#include <QtDebug>
#include <QStyle>
#include <QFont>
#include <QShortcut>

WSearchLineEdit::WSearchLineEdit(QWidget* pParent)
        : QLineEdit(pParent),
          WBaseWidget(this) {
    setAcceptDrops(false);
    m_clearButton = new QToolButton(this);
    QPixmap pixmap(":/skins/cross.png");
    m_clearButton->setIcon(QIcon(pixmap));
    m_clearButton->setIconSize(pixmap.size());
    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    m_clearButton->hide();

    m_place = true;
    showPlaceholder();

    QShortcut *setFocusShortcut = new QShortcut(
        QKeySequence(tr("Ctrl+F", "Search|Focus")), this);
    connect(setFocusShortcut, SIGNAL(activated()),
            this, SLOT(setFocus()));
    QShortcut *clearTextShortcut = new QShortcut(
        QKeySequence(tr("Esc", "Search|Clear")), this, 0, 0,
        Qt::WidgetShortcut);
    connect(clearTextShortcut, SIGNAL(activated()),
            this, SLOT(clear()));

    // Set up a timer to search after a few hundred milliseconds timeout.  This
    // stops us from thrashing the database if you type really fast.
    m_searchTimer.setSingleShot(true);
    connect(&m_searchTimer, SIGNAL(timeout()),
            this, SLOT(triggerSearch()));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotSetupTimer(const QString&)));

    // When you hit enter, it will trigger the search.
    connect(this, SIGNAL(returnPressed()),
            this, SLOT(triggerSearch()));

    connect(m_clearButton, SIGNAL(clicked()),
            this, SLOT(clear()));
    // Forces immediate update of tracktable
    connect(m_clearButton, SIGNAL(clicked()),
            this, SLOT(triggerSearch()));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(updateCloseButton(const QString&)));

    // The width of the frame for the widget based on the styling.
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    // Ensures the text does not obscure the clear image.
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").
                  arg(m_clearButton->sizeHint().width() + frameWidth + 1));
}

WSearchLineEdit::~WSearchLineEdit() {
}

void WSearchLineEdit::setup(QDomNode node, const SkinContext& context) {
    // Background color
    QColor bgc(255,255,255);
    if (context.hasNode(node, "BgColor")) {
        bgc.setNamedColor(context.selectString(node, "BgColor"));
        setAutoFillBackground(true);
    }
    QPalette pal = palette();
    pal.setBrush(backgroundRole(), WSkinColor::getCorrectColor(bgc));

    // Foreground color
    m_fgc = QColor(0,0,0);
    if (context.hasNode(node, "FgColor")) {
        m_fgc.setNamedColor(context.selectString(node, "FgColor"));
    }
    bgc = WSkinColor::getCorrectColor(bgc);
    m_fgc = QColor(255 - bgc.red(), 255 - bgc.green(), 255 - bgc.blue());
    pal.setBrush(foregroundRole(), m_fgc);
    setPalette(pal);
}

void WSearchLineEdit::resizeEvent(QResizeEvent* e) {
    QLineEdit::resizeEvent(e);
    QSize sz = m_clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    int height = (rect().bottom() + 1 - sz.height())/2;
    if (layoutDirection() == Qt::LeftToRight) {
        m_clearButton->move(rect().right() - frameWidth - sz.width() - 1,
                            height);
    } else {
        m_clearButton->move(frameWidth + 1, height);
    }
}

void WSearchLineEdit::focusInEvent(QFocusEvent* event) {
    QLineEdit::focusInEvent(event);
    if (m_place) {
        // This gets rid of the blue mac highlight.
        setAttribute(Qt::WA_MacShowFocusRect, false);
        //Must block signals here so that we don't emit a search() signal via
        //textChanged().
        blockSignals(true);
        setText("");
        blockSignals(false);
        QPalette pal = palette();
        pal.setColor(foregroundRole(), m_fgc);
        setPalette(pal);
        m_place = false;
        emit(searchStarting());
    }
}

void WSearchLineEdit::focusOutEvent(QFocusEvent* event) {
    QLineEdit::focusOutEvent(event);
    if (text().isEmpty()) {
        m_place = true;
        showPlaceholder();
        emit(searchCleared());
    } else {
        m_place = false;
    }
}

// slot
void WSearchLineEdit::restoreSearch(const QString& text) {
    if(text.isNull()) {
        // disable
        setEnabled(false);
        blockSignals(true);
        setText("- - -");
        blockSignals(false);
        return;
    }
    setEnabled(true);
    qDebug() << "WSearchLineEdit::restoreSearch(" << text << ")";
    blockSignals(true);
    setText(text);
    blockSignals(false);
    if (text == "") {
        m_place = true;
        showPlaceholder();
    } else {
        QPalette pal = palette();
        pal.setColor(foregroundRole(), m_fgc);
        setPalette(pal);
        m_place = false;
    }
    updateCloseButton(text);
}

void WSearchLineEdit::slotSetupTimer(const QString& text)
{
    Q_UNUSED(text);
    m_searchTimer.stop();
    //300 milliseconds timeout
    m_searchTimer.start(300);
}

void WSearchLineEdit::triggerSearch()
{
    m_searchTimer.stop();
    emit(search(text()));
}

void WSearchLineEdit::showPlaceholder() {
    //Must block signals here so that we don't emit a search() signal via
    //textChanged().
    blockSignals(true);
    setText(tr("Search..."));
    blockSignals(false);
    QPalette pal = palette();
    pal.setColor(foregroundRole(), Qt::lightGray);
    setPalette(pal);
}

void WSearchLineEdit::updateCloseButton(const QString& text)
{
    m_clearButton->setVisible(!text.isEmpty() && !m_place);
}

bool WSearchLineEdit::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QLineEdit::event(pEvent);
}
