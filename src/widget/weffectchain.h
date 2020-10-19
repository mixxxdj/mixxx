#ifndef WEFFECTCHAIN_H
#define WEFFECTCHAIN_H

#include <QWidget>
#include <QLabel>
#include <QDomNode>

#include "effects/effectchain.h"
#include "widget/wlabel.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectChain : public WLabel {
    Q_OBJECT
  public:
    WEffectChain(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void chainUpdated();

  private:
    // Set the EffectChain that should be monitored by this WEffectChain
    void setEffectChain(EffectChainPointer pEffectChain);

    EffectsManager* m_pEffectsManager;
    EffectChainPointer m_pEffectChain;
};

#endif /* WEFFECTCHAIN_H */
