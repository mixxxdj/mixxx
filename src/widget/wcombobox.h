#pragma once

#include <QComboBox>
#include <QDomNode>
#include <QEvent>

#include "widget/wbasewidget.h"
#include "skin/skincontext.h"

class WComboBox : public QComboBox, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WComboBox(QWidget* pParent);

    void setup(const QDomNode& node, const SkinContext& context);

    void onConnectedControlChanged(double dParameter, double dValue) override;

  protected:
    bool event(QEvent* pEvent) override;

  private slots:
    void slotCurrentIndexChanged(int index);
};
