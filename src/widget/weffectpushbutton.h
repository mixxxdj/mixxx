#pragma once

#include "effects/defs.h"
#include "widget/wpushbutton.h"

class QAction;
class QMenu;
class EffectsManager;

class WEffectPushButton : public WPushButton {
    Q_OBJECT
  public:
    WEffectPushButton(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue) override;

  protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

  private slots:
    void parameterUpdated();
    void slotActionChosen(QAction* action);

  private:
    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
    QMenu* m_pButtonMenu;
};
