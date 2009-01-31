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

	connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clear()));
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
		setText("");
		setPaletteForegroundColor(Qt::black);
		m_place = false;
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

void WSearchLineEdit::showPlaceholder() {
	setText("Search...");
	setPaletteForegroundColor(Qt::lightGray);
}

void WSearchLineEdit::updateCloseButton(const QString& text)
{
    m_clearButton->setVisible(!text.isEmpty() && !m_place);
}
