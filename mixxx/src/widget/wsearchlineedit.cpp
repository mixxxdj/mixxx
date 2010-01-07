#include "wwidget.h"
#include "wskincolor.h"
#include "wsearchlineedit.h"

#include <QtDebug>
#include <QStyle>
#include <QFont>

WSearchLineEdit::WSearchLineEdit(QString& skinpath, QDomNode node, QWidget* parent) : QLineEdit(parent) {

	m_clearButton = new QToolButton(this);
	QPixmap pixmap(skinpath.append("/skins/cross.png"));
	m_clearButton->setIcon(QIcon(pixmap));
	m_clearButton->setIconSize(pixmap.size());
	m_clearButton->setCursor(Qt::ArrowCursor);
	m_clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
	m_clearButton->hide();

	m_place = true;
	showPlaceholder();

    setup(node);

	//Set up a timer to search after a few hundred milliseconds timeout.
	//This stops us from thrashing the database if you type really fast.
	m_searchTimer.setSingleShot(true);
	connect(&m_searchTimer, SIGNAL(timeout()),
          this, SLOT(triggerSearch()));

    connect(this, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotSetupTimer(const QString&)));

	//When you hit enter, it will trigger the search.
	connect(this, SIGNAL(returnPressed()), this, SLOT(triggerSearch()));

	connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    //Forces immediate update of tracktable
	connect(m_clearButton, SIGNAL(clicked()), this, SLOT(triggerSearch()));

	connect(this, SIGNAL(textChanged(const QString&)),
          this, SLOT(updateCloseButton(const QString&)));

	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").
                arg(m_clearButton->sizeHint().width() + frameWidth + 1));

    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(),
                        m_clearButton->sizeHint().height() + frameWidth * 2 + 2),
                    qMax(msz.height(),
                        m_clearButton->sizeHint().height() + frameWidth * 2 + 2));
}

void WSearchLineEdit::setup(QDomNode node)
{
    // Background color
    QColor bgc(255,255,255);
    if (!WWidget::selectNode(node, "BgColor").isNull()) {
        bgc.setNamedColor(WWidget::selectNodeQString(node, "BgColor"));
        this->setAutoFillBackground(true);
    }
    QPalette palette;
    palette.setBrush(this->backgroundRole(), WSkinColor::getCorrectColor(bgc));

    // Foreground color
    m_fgc = QColor(0,0,0);
    if (!WWidget::selectNode(node, "FgColor").isNull()) {
        m_fgc.setNamedColor(WWidget::selectNodeQString(node, "FgColor"));
    }
    bgc = WSkinColor::getCorrectColor(bgc);
    m_fgc = QColor(255 - bgc.red(), 255 - bgc.green(), 255 - bgc.blue());
    palette.setBrush(this->foregroundRole(), m_fgc);
    this->setPalette(palette);

}

void WSearchLineEdit::resizeEvent(QResizeEvent* e)
{
    QSize sz = m_clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    m_clearButton->move(rect().right() - frameWidth - sz.width(),
                      (rect().bottom() + 1 - sz.height())/2);
}

void WSearchLineEdit::focusInEvent(QFocusEvent* event) {
    if (m_place) {
        //Must block signals here so that we don't emit a search() signal via
        //textChanged().
        blockSignals(true);
        setText("");
        blockSignals(false);
        QPalette palette = this->palette();
        palette.setColor(this->foregroundRole(), m_fgc);
        setPalette(palette);
        m_place = false;
        emit(searchStarting());
    }
}

void WSearchLineEdit::focusOutEvent(QFocusEvent* event) {
    if (text().isEmpty()) {
        m_place = true;
        showPlaceholder();
    } else {
        m_place = false;
    }
}

void WSearchLineEdit::restoreSearch(const QString& text) {
    qDebug() << "WSearchLineEdit::restoreSearch(" << text << ")";
    blockSignals(true);
    setText(text);
    blockSignals(false);
    if (text == "") {
        m_place = true;
        showPlaceholder();
    } else {
        QPalette palette = this->palette();
        palette.setColor(this->foregroundRole(), m_fgc);
        setPalette(palette);
        m_place = false;
    }
    updateCloseButton(text);
}

void WSearchLineEdit::slotSetupTimer(const QString& text)
{
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
    QPalette palette = this->palette();
    palette.setColor(this->foregroundRole(), Qt::lightGray);
    setPalette(palette);
    emit(searchCleared());
}

void WSearchLineEdit::updateCloseButton(const QString& text)
{
    m_clearButton->setVisible(!text.isEmpty() && !m_place);
}
