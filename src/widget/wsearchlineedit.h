#ifndef WSEARCHLINEEDIT_H
#define WSEARCHLINEEDIT_H

#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QTimer>
#include <QDomNode>
#include <QColor>
#include <QEvent>

#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "widget/wbasewidget.h"

class WSearchLineEdit : public QLineEdit, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WSearchLineEdit(QWidget* pParent);

    void setup(const QDomNode& node, const SkinContext& context);

  protected:
    void resizeEvent(QResizeEvent* /*unused*/) override;
    void focusInEvent(QFocusEvent* /*unused*/) override;
    void focusOutEvent(QFocusEvent* /*unused*/) override;
    bool event(QEvent* pEvent) override;

  signals:
    void search(const QString& text);
    void searchCleared();
    void searchStarting();

  public slots:
    void restoreSearch(const QString& text);
    void slotTextChanged(const QString& text);

  private slots:
    void updateCloseButton(const QString& text);
    void slotSetupTimer(const QString& text);
    void triggerSearch();
    void onSearchTextCleared();

  private:
    void showPlaceholder();

    QTimer m_searchTimer;
    QToolButton* m_clearButton;
    bool m_place;
    QColor m_fgc; //Foreground color
};

#endif
