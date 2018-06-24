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

#include "library/dao/savedqueriesdao.h"
#include "library/libraryfeature.h"
#include "library/trackcollection.h"
#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "widget/wbasewidget.h"

class WSearchLineEdit : public QLineEdit, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WSearchLineEdit(QWidget* pParent = nullptr);

    void setup(const QDomNode& node, const SkinContext& context);
    void restoreSearch(const QString& text,
            QPointer<LibraryFeature> pFeature = nullptr);
    void slotRestoreSaveButton();
    void setTrackCollection(TrackCollection* pTrackCollection);

  protected:
    void resizeEvent(QResizeEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool event(QEvent* pEvent) override;

  signals:
    void search(const QString& text);
    void searchCleared();
    void searchStarting();
    void focused();
    void cancel();

  private slots:
    void slotTextChanged(const QString& text);
    void updateButtons(const QString& text);
    void slotSetupTimer(const QString&);
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
    QPointer<TrackCollection> m_pTrackCollection;
};

#endif
