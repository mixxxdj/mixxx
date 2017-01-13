#ifndef WLIBRARYSIDEBAREXPANDED_H
#define WLIBRARYSIDEBAREXPANDED_H
#include <QStackedWidget>
#include <QMutex>
#include <QHash>

#include "widget/wbasewidget.h"

class LibraryFeature;

class WBaseLibrary : public QStackedWidget, public WBaseWidget
{
    Q_OBJECT
  public:

    WBaseLibrary(QWidget* parent = nullptr);

    virtual bool registerView(LibraryFeature* pFeature, QWidget* view);
    
    LibraryFeature* getCurrentFeature();
    
    Q_PROPERTY(int showFocus READ getShowFocus WRITE setShowFocus)
    
    int getShowFocus();
    
    // Sets the widget to the focused state, it's not the same as Qt focus
    void setShowFocus(int sFocus);
    
    virtual void switchToFeature(LibraryFeature* pFeature);
    virtual void search(const QString&) {}
    virtual void searchStarting() {}
    virtual void searchCleared() {}

  signals:

    void focused();
    void collapsed();
    void uncollapsed();

  protected:
      
    bool eventFilter(QObject*, QEvent* pEvent) override;
    bool event(QEvent* pEvent) override;
    void resizeEvent(QResizeEvent* pEvent) override;

    QHash<LibraryFeature*, QWidget*> m_viewsByFeature;

  private:
    
    LibraryFeature* m_pCurrentFeature;
    int m_showFocus;
    bool m_isCollapsed;
};

#endif // WLIBRARYSIDEBAREXPANDED_H
