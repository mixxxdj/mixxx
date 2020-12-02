#ifndef WEFFECTPARAMETERBASE_H
#define WEFFECTPARAMETERBASE_H

#include <QByteArrayData>
#include <QDomNode>
#include <QString>

#include "effects/defs.h"
#include "effects/effectparameterslotbase.h"
#include "skin/skincontext.h"
#include "widget/wlabel.h"

class EffectsManager;
class QObject;
class QWidget;

class WEffectParameterBase : public WLabel {
    Q_OBJECT
  public:
    WEffectParameterBase(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override = 0;

  protected slots:
    void parameterUpdated();

  protected:
    // Set the EffectParameterSlot that should be monitored by this
    // WEffectParameterBase.
    void setEffectParameterSlot(EffectParameterSlotBasePointer pEffectParameterSlot);

    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
};

#endif /* WEFFECTPARAMETERBASE_H */
