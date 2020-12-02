#ifndef WEFFECTPUSHBUTTON_H
#define WEFFECTPUSHBUTTON_H

#include <QAction>
#include <QByteArrayData>
#include <QDomNode>
#include <QMenu>
#include <QMouseEvent>
#include <QString>
#include <QWidget>

#include "effects/defs.h"
#include "effects/effectsmanager.h"
#include "skin/skincontext.h"
#include "widget/wpushbutton.h"

class ConfigKey;
class EffectsManager;
class QAction;
class QMenu;
class QMouseEvent;
class QObject;
class QWidget;

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

#endif // WEFFECTPUSHBUTTON_H
