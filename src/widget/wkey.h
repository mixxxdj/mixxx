#pragma once

#include "control/controlproxy.h"
#include "preferences/colorpalettesettings.h"
#include "widget/wlabel.h"

class WKey : public WLabel  {
    Q_OBJECT
  public:
    explicit WKey(const QString& group, UserSettingsPointer pConfig, QWidget* pParent = nullptr);

    void onConnectedControlChanged(double dParameter, double dValue) override;
    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void setValue(double dValue);
    void keyNotationChanged(double dValue);
    void setCents();

  private:
    double m_dOldValue;
    bool m_displayCents;
    bool m_displayKey;
    ControlProxy m_keyNotation;
    ControlProxy m_engineKeyDistance;
    ColorPaletteSettings m_colorPaletteSettings;
    mixxx::track::io::key::ChromaticKey key;
    void paintEvent(QPaintEvent* event) override;
};
