#pragma once

#include <QDomNode>

#include "effects/effectparameterslotbase.h"
#include "skin/legacy/skincontext.h"
#include "widget/wlabel.h"

class EffectsManager;

class WEffectParameterNameBase : public WLabel {
    Q_OBJECT
  public:
    WEffectParameterNameBase(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override = 0;

    void mousePressEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

  protected slots:
    void parameterUpdated();

  protected:
    void setEffectParameterSlot(EffectParameterSlotBasePointer pEffectKnobParameterSlot);

    EffectsManager* m_pEffectsManager;
    EffectSlotPointer m_pEffectSlot;
    EffectParameterSlotBasePointer m_pParameterSlot;

  private:
    const QString mimeTextIdentifier() const;
};
