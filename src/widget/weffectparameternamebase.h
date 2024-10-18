#pragma once

#include <QTimer>

#include "effects/defs.h"
#include "widget/wlabel.h"

class EffectsManager;

class WEffectParameterNameBase : public WLabel {
    Q_OBJECT
  public:
    WEffectParameterNameBase(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override = 0;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    QSize sizeHint() const override;

  protected slots:
    void parameterUpdated();
    void showNewValue(double v);

  protected:
    void setEffectParameterSlot(EffectParameterSlotBasePointer pEffectKnobParameterSlot);

    EffectsManager* m_pEffectsManager;
    EffectSlotPointer m_pEffectSlot;
    EffectParameterSlotBasePointer m_pParameterSlot;

  private:
    const QString mimeTextIdentifier() const;
    QString m_unitString;
    QString m_text;
    QTimer m_displayNameResetTimer;
    bool m_parameterUpdated;
    int m_widthHint;
};
