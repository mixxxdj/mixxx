#include "wsearchlineedit.h"

#include <QStyle>
#include <QFont>

WSearchLineEdit::WSearchLineEdit(QString& skinpath, QWidget* parent) : QLineEdit(parent) {

	m_clearButton = new QToolButton(this);
	QPixmap pixmap(skinpath.append("/skins/cross.png"));
	m_clearButton->setIcon(QIcon(pixmap));
	m_clearButton->setIconSize(pixmap.size());
	m_clearButton->setCursor(Qt::ArrowCursor);
	m_clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
	m_clearButton->hide();

	m_place = true;
	showPlaceholder();
	
	//Set up a timer to search after a few hundred milliseconds timeout.
	//This stops us from thrashing the database if you type really fast.
	m_searchTimer.setSingleShot(true);
	connect(&m_searchTimer, SIGNAL(timeout()), this, SLOT(triggerSearch()));
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(slotSetupTimer(const QString&)));
	//When you hit enter, it will trigger the search.
	connect(this, SIGNAL(returnPressed()), this, SLOT(triggerSearch()));

	connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clear()));
	connect(m_clearButton, SIGNAL(clicked()), this, SLOT(triggerSearch())); //Forces immediate update of tracktable
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateCloseButton(const QString&)));
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(m_clearButton->sizeHint().width() + frameWidth + 1));
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), m_clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), m_clearButton->sizeHint().height() + frameWidth * 2 + 2));
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
		//Must block signals here so that we don't emit a search() signal via textChanged().
		blockSignals(true);
		setText("");
		blockSignals(false);
		QPalette palette = this->palette();
		palette.setColor(this->foregroundRole(), Qt::black);
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

void WSearchLineEdit::slotSetupTimer(const QString& text)
{
	m_searchTimer.stop();
	m_searchTimer.start(300); //300 milliseconds timeout
	connect(&m_searchTimer, SIGNAL(timeout()), this, SLOT(triggerSearch()));
}

void WSearchLineEdit::triggerSearch()
{
	m_searchTimer.stop();
	emit(search(this->text()));
}

void WSearchLineEdit::showPlaceholder() {
	//Must block signals here so that we don't emit a search() signal via textChanged().
	blockSignals(true);
	setText("Search...");
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
