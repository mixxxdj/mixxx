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
    
  signals:
  
    void focused();
    void collapsed();
    void uncollapsed();    
    
  public slots:

    virtual void switchToFeature(LibraryFeature* pFeature);

  protected:
      
    bool eventFilter(QObject*, QEvent* pEvent);
    bool event(QEvent* pEvent) override;
    void resizeEvent(QResizeEvent* pEvent);

    QHash<LibraryFeature*, QWidget*> m_featureMap;

  private:
    
    LibraryFeature* m_pCurrentFeature;
    QMutex m_mutex;
    int m_showFocus;
    bool m_isCollapsed;
};

#endif // WLIBRARYSIDEBAREXPANDED_H
