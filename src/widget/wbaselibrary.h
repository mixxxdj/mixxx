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

  public slots:

    virtual void switchToView(const QString& name);

  protected:

    bool event(QEvent* pEvent) override;

    QMap<QString, QWidget*> m_viewMap;

  private:

    QMutex m_mutex;
};

#endif // WLIBRARYSIDEBAREXPANDED_H
