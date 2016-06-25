#ifndef WLIBRARYSIDEBAREXPANDED_H
#define WLIBRARYSIDEBAREXPANDED_H
#include <QStackedWidget>
#include <QMutex>
#include <QMap>

#include "widget/wbasewidget.h"

class WBaseLibrary : public QStackedWidget, public WBaseWidget
{
    Q_OBJECT
  public:

    WBaseLibrary(QWidget* parent = nullptr);

    virtual bool registerView(QString name, QWidget* view);
    
    QString getCurrentViewName();
    
    Q_PROPERTY(int showFocus READ getShowFocus WRITE setShowFocus)
    
    int getShowFocus();
    
    // Sets the widget to the focused state, it's not the same as Qt focus
    void setShowFocus(int sFocus);
    
  signals:
  
    void focused();
    
  public slots:

    virtual void switchToView(const QString& name);

  protected:
      
    bool eventFilter(QObject*, QEvent* pEvent);
    
    bool event(QEvent* pEvent) override;

    QMap<QString, QWidget*> m_viewMap;

  private:
    
    QString m_currentViewName;

    QMutex m_mutex;
    
    int m_showFocus;
};

#endif // WLIBRARYSIDEBAREXPANDED_H
