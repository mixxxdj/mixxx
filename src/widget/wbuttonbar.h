#ifndef WBUTTONBAR_H
#define WBUTTONBAR_H

#include <QLayout>
#include <QVariant>
#include <QIcon>
#include <QPushButton>
#include <QButtonGroup>

#include "widget/wwidget.h"

class WButtonBar : WWidget
{
    Q_OBJECT
  public:
    WButtonBar(QWidget* parent = nullptr);

  signals:

    void buttonClicked(QVariant data);

  private slots:
    
    void slotButtonClicked(int id);

  private:

    QLayout* m_pLayout;
    QButtonGroup* m_pButtonGroup;
    QList<QVariant> m_data;
};

#endif // WBUTTONBAR_H
