#ifndef WBUTTONBAR_H
#define WBUTTONBAR_H

#include <QLayout>
#include <QVariant>
#include <QIcon>
#include <QPushButton>
#include <QFrame>

#include "widget/wfeatureclickbutton.h"

class WButtonBar : public QFrame
{
    Q_OBJECT
  public:
    WButtonBar(QWidget* parent = nullptr);

    WFeatureClickButton* addButton(LibraryFeature *pFeature);

  private:

    QLayout* m_pLayout;
};

#endif // WBUTTONBAR_H
