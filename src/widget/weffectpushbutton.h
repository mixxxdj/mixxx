#ifndef WEFFECTPUSHBUTTON_H
#define WEFFECTPUSHBUTTON_H

#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QDomNode>
#include <QWidget>

#include "widget/wpushbutton.h"
#include "effects/effectsmanager.h"
#include "skin/skincontext.h"

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

#endif // WEFFECTPUSHBUTTON_H
