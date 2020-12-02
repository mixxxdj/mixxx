#ifndef WEFFECT_H
#define WEFFECT_H

#include <QByteArrayData>
#include <QDomNode>
#include <QString>

#include "effects/defs.h"
#include "effects/effectslot.h"
#include "skin/skincontext.h"
#include "widget/wlabel.h"

class EffectsManager;
class QObject;
class QWidget;

class WEffect : public WLabel {
    Q_OBJECT
  public:
    WEffect(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void effectUpdated();

  private:
    // Set the EffectSlot that should be monitored by this WEffect.
    void setEffectSlot(EffectSlotPointer pEffectSlot);

    EffectsManager* m_pEffectsManager;
    EffectSlotPointer m_pEffectSlot;
};


#endif /* WEFFECT_H */
