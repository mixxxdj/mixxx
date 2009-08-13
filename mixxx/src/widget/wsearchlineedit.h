#ifndef WSEARCHLINEEDIT_H
#define WSEARCHLINEEDIT_H

#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QTimer>

class WSearchLineEdit : public QLineEdit {

	Q_OBJECT

public:
	WSearchLineEdit(QString& skinpath, QWidget* parent = 0);

protected:
    void resizeEvent(QResizeEvent*);
	virtual void focusInEvent(QFocusEvent*);
	virtual void focusOutEvent(QFocusEvent*);

signals:
	void search(const QString& text);
	void searchCleared();
	void searchStarting();

private slots:
    void updateCloseButton(const QString& text);
    void slotSetupTimer(const QString& text);
	void triggerSearch();

private:
	void showPlaceholder();

	QTimer m_searchTimer;
	QToolButton* m_clearButton;
	bool m_place;
};

#endif
