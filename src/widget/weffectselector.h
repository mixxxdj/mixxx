#ifndef WEFFECTSELECTOR_H
#define WEFFECTSELECTOR_H

#include <QDomNode>
#include <QComboBox>
#include "effects/effectrack.h"
#include "effects/effectslot.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectSelector : public QComboBox, public WBaseWidget {
    Q_OBJECT
  public:
    WEffectSelector(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context);

  private slots:
    void slotEffectUpdated();
    void slotEffectSelected(int newIndex);

  private:
    EffectsManager* m_pEffectsManager;
    EffectSlotPointer m_pEffectSlot;
    EffectChainSlotPointer m_pChainSlot;
    EffectRackPointer m_pRack;
};


#endif /* WEFFECTSELECTOR_H */
