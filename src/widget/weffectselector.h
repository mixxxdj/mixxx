#ifndef WEFFECTSELECTOR_H
#define WEFFECTSELECTOR_H

#include <QByteArrayData>
#include <QComboBox>
#include <QDomNode>
#include <QString>

#include "effects/defs.h"
#include "effects/effectrack.h"
#include "effects/effectslot.h"
#include "skin/skincontext.h"
#include "widget/wbasewidget.h"

class EffectsManager;
class QEvent;
class QObject;
class QWidget;

class WEffectSelector : public QComboBox, public WBaseWidget {
    Q_OBJECT
  public:
    WEffectSelector(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context);

  protected:
    bool event(QEvent* pEvent) override;

  private slots:
    void slotEffectUpdated();
    void slotEffectSelected(int newIndex);
    void populate();

  private:
    EffectsManager* m_pEffectsManager;
    EffectSlotPointer m_pEffectSlot;
    EffectChainSlotPointer m_pChainSlot;
    EffectRackPointer m_pRack;
    double m_scaleFactor;
};


#endif /* WEFFECTSELECTOR_H */
