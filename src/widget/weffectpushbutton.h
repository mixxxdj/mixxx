#ifndef WEFFECTPUSHBUTTON_H
#define WEFFECTPUSHBUTTON_H

#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QDomNode>
#include <QWidget>

#include "widget/wpushbutton.h"
#include "skin/skincontext.h"
#include "effects/effectsmanager.h"

class WEffectPushButton : public WPushButton {
    Q_OBJECT
  public:
    WEffectPushButton(QWidget* pParent, EffectsManager* pEffectsManager);
    virtual ~WEffectPushButton();

    virtual void setup(QDomNode node, const SkinContext& context);

  public slots:
    virtual void onConnectedControlChanged(double dParameter, double dValue);

  protected:
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);

  private slots:
    void parameterUpdated();
    void slotActionChosen(QAction* action);

  private:
    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
    QMenu* m_pButtonMenu;
};

#endif // WEFFECTPUSHBUTTON_H
