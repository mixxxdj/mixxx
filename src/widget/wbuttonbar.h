#ifndef WBUTTONBAR_H
#define WBUTTONBAR_H

#include <QLayout>
#include <QVariant>
#include <QIcon>
#include <QPushButton>

#include "widget/wwidget.h"
#include "widget/wrightclickbutton.h"

class WButtonBar : public WWidget
{
    Q_OBJECT
  public:
    WButtonBar(QWidget* parent = nullptr);

    WRightClickButton* addButton(const QIcon& icon, const QVariant& title);

  private:

    QLayout* m_pLayout;
};

#endif // WBUTTONBAR_H
