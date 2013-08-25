#ifndef WEFFECTCHAIN_H
#define WEFFECTCHAIN_H

#include <QWidget>
#include <QLabel>

#include "effects/effectchainslot.h"

class WEffectChain : public QLabel {
    Q_OBJECT
  public:
    WEffectChain(QWidget* pParent=NULL);
    virtual ~WEffectChain();

    // Set the EffectChain that should be monitored by this WEffectChain
    void setEffectChainSlot(EffectChainSlotPointer pEffectChainSlot);

  private slots:
    void chainUpdated();

  private:
    EffectChainSlotPointer m_pEffectChainSlot;
};

#endif /* WEFFECTCHAIN_H */
