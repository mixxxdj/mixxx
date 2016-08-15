#ifndef WSEARCHLINEEDIT_H
#define WSEARCHLINEEDIT_H

#include <QColor>
#include <QDomNode>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QModelIndex>
#include <QTimer>
#include <QToolButton>

#include "library/libraryfeature.h"
#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "widget/wbasewidget.h"

class WSearchLineEdit : public QLineEdit, public WBaseWidget {
    Q_OBJECT
  public:
    
    explicit WSearchLineEdit(QWidget* pParent = nullptr);

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
    void restoreSearch(const QString& text, QPointer<LibraryFeature> pFeature = nullptr);
    void slotTextChanged(const QString& text);
    void slotRestoreSaveButton();

  private slots:
    void updateButtons(const QString& text);
    void slotSetupTimer(const QString& text);
    void triggerSearch();
    void onSearchTextCleared();
    
    void saveQuery();
    void restoreQuery();

  private:
    void showPlaceholder();

    QPointer<LibraryFeature> m_pCurrentFeature;
    QTimer m_searchTimer;
    QToolButton* m_pClearButton;
    QToolButton* m_pSaveButton;
    QToolButton* m_pDropButton;
    bool m_place;
    QColor m_fgc; //Foreground color
};

#endif
