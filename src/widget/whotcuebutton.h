#pragma once

#include <QDomNode>
#include <QMouseEvent>
#include <QWidget>

#include "skin/skincontext.h"
#include "util/parented_ptr.h"
#include "widget/wcuemenupopup.h"
#include "widget/wpushbutton.h"

class WHotcueButton : public WPushButton {
    Q_OBJECT
  public:
    WHotcueButton(QWidget* pParent);

    void setup(const QDomNode& node, const SkinContext& context) override;

  protected:
    void mousePressEvent(QMouseEvent* e) override;

  private slots:
    void slotColorChanged(double color);

  private:
    ConfigKey createConfigKey(const QString& name);

    QString m_group;
    int m_hotcue;
    QColor m_cueColor;
    parented_ptr<ControlProxy> m_pCoColor;
    parented_ptr<WCueMenuPopup> m_pCueMenuPopup;
};
