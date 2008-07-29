#ifndef WSEARCHLINEEDIT_H
#define WSEARCHLINEEDIT_H

#include <QLineEdit>
#include <QToolButton>

class WSearchLineEdit : public QLineEdit {

	Q_OBJECT

public:
	WSearchLineEdit(QString& skinpath, QWidget* parent = 0);

protected:
    void resizeEvent(QResizeEvent*);

private slots:
    void updateCloseButton(const QString& text);

private:
	QToolButton* m_clearButton;
};

#endif