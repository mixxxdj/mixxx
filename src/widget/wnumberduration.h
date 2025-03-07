#pragma once

#include "preferences/dialog/dlgprefdeck.h"
#include "skin/legacy/skincontext.h"
#include "wnumber.h"

class ControlProxy;

class WNumberDuration : public WNumber {
    Q_OBJECT

  public:
    explicit WNumberDuration(QWidget* parent = nullptr);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void setValue(double dValue) override;

  private:
    TrackTime::DisplayFormat m_displayFormat;
    QString m_displayFormatString;
};
