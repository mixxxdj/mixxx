#ifndef WEFFECTCHAIN_H
#define WEFFECTCHAIN_H

#include <QByteArrayData>
#include <QDomNode>
#include <QLabel>
#include <QString>
#include <QWidget>

#include "effects/defs.h"
#include "effects/effectchainslot.h"
#include "skin/skincontext.h"
#include "widget/wlabel.h"

class EffectsManager;
class QObject;
class QWidget;

class WEffectChain : public WLabel {
    Q_OBJECT
  public:
    WEffectChain(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void chainUpdated();

  private:
    // Set the EffectChain that should be monitored by this WEffectChain
    void setEffectChainSlot(EffectChainSlotPointer pEffectChainSlot);

    EffectsManager* m_pEffectsManager;
    EffectChainSlotPointer m_pEffectChainSlot;
};

#endif /* WEFFECTCHAIN_H */
