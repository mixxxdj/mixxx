#ifndef WCOMBOBOX_H
#define WCOMBOBOX_H

#include <QByteArrayData>
#include <QComboBox>
#include <QDomNode>
#include <QEvent>
#include <QString>

#include "skin/skincontext.h"
#include "widget/wbasewidget.h"

class QEvent;
class QObject;
class QWidget;

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

#endif /* WCOMBOBOX_H */
