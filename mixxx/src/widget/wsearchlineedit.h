#ifndef WSEARCHLINEEDIT_H
#define WSEARCHLINEEDIT_H

#include <QLineEdit>
#include <QToolButton>
#include <QLabel>

class WSearchLineEdit : public QLineEdit {

	Q_OBJECT

public:
	WSearchLineEdit(QString& skinpath, QWidget* parent = 0);

protected:
    void resizeEvent(QResizeEvent*);
	virtual void focusInEvent(QFocusEvent*);
	virtual void focusOutEvent(QFocusEvent*);

private slots:
    void updateCloseButton(const QString& text);

private:
	void showPlaceholder();

	QToolButton* m_clearButton;
	bool m_place;
};

#endif
