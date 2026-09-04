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
    /// Returns the menu pointer.
    QMenu* createConnectAndGetMenu();
    /// Returns the menu pointer if it has been created, or nullptr
    QMenu* getMenuIfCreated();
    void setCheckedActionByValue(double value);

    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
    /// Note: the menu should not be used directly since it is created only on
    /// demand to reduce skin loading time.
    /// Use getMenuIfCreated() or createConnectAndGetMenu() instead.
    QMenu* m_pButtonMenu;
};
