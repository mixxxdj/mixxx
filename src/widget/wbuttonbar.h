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

  signals:
    void ensureVisible(QWidget* widget);

  protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;

  private:
    QLayout* m_pLayout;
    int m_focusItem;
};

#endif // WBUTTONBAR_H
