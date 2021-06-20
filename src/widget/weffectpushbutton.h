#pragma once

#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QDomNode>
#include <QWidget>

#include "widget/wpushbutton.h"
#include "effects/effectsmanager.h"
#include "effects/defs.h"
#include "skin/legacy/skincontext.h"

class WEffectPushButton : public WPushButton {
    Q_OBJECT
  public:
    WEffectPushButton(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;
    void setupEffectParameterSlot(const ConfigKey& configKey);

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue) override;

  protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

  private slots:
    void parameterUpdated();
    void slotActionChosen(QAction* action);

  private:
    // Set the EffectParameterSlot that should be monitored by this
    // WEffectKnobComposed.
    void setEffectParameterSlot(EffectButtonParameterSlotPointer pParameterSlot);


    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
    QMenu* m_pButtonMenu;
};
