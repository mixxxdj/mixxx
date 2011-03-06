#ifndef WEFFECTCHAIN_H
#define WEFFECTCHAIN_H

#include <QWidget>
#include <QLabel>

#include "effects/effectchain.h"

class WEffectChain : public QLabel {
    Q_OBJECT
  public:
    WEffectChain(QWidget* pParent=NULL);
    virtual ~WEffectChain();

    // Set the EffectChain that should be monitored by this WEffectChain
    void setEffectChain(EffectChainPointer effectChain);

  private slots:
    void chainUpdated();

  private:
    EffectChainPointer m_pEffectChain;
};

#endif /* WEFFECTCHAIN_H */
