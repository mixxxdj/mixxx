#ifndef WCOMBOBOX_H
#define WCOMBOBOX_H

#include <QComboBox>
#include <QDomNode>
#include <QEvent>

#include "widget/wbasewidget.h"
#include "skin/skincontext.h"

class WComboBox : public QComboBox, public WBaseWidget {
    Q_OBJECT
  public:
    WComboBox(QWidget* pParent);
    virtual ~WComboBox();

    void setup(QDomNode node, const SkinContext& context);

    void onConnectedControlValueChanged(double v);

  protected:
    bool event(QEvent* pEvent);

  private slots:
    void slotCurrentIndexChanged(int index);
};

#endif /* WCOMBOBOX_H */
