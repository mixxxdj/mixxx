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

    Q_PROPERTY(QColor light MEMBER m_lightTextColor WRITE setLightTextColorChanged);
    Q_PROPERTY(QColor dark MEMBER m_darkTextColor WRITE setDarkTextColorColorChanged);

  protected:
    void mousePressEvent(QMouseEvent* e) override;

  private slots:
    void slotColorChanged(double color);

  private:
    ConfigKey createConfigKey(const QString& name);
    void setLightTextColorChanged(QColor color);
    void setDarkTextColorColorChanged(QColor color);
    void updateStyleSheet();

    QString m_group;
    int m_hotcue;
    bool m_hoverCueColor;
    parented_ptr<ControlProxy> m_pCoColor;
    parented_ptr<WCueMenuPopup> m_pCueMenuPopup;
    QColor m_cueColor;
    QColor m_lightTextColor;
    QColor m_darkTextColor;
};
